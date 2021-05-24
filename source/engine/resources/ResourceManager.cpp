#include "ResourceManager.hpp"

#include <utility>

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"
#include "fmt/format.h"
#include "glm/gtc/type_ptr.hpp"
#include "nlohmann/json.hpp"

#include "Shader.hpp"
#include "../Application.hpp"

using json = nlohmann::json;


namespace engine {

    ResourceManager::ResourceManager(std::shared_ptr<engine::Device> device, vk::Queue graphicsQueue)
            : m_device(std::move(device)), m_graphicsQueue(graphicsQueue) {
        createDescriptorSetLayout();
        createDescriptorPool();
    }

    ResourceManager::~ResourceManager() = default;

    void ResourceManager::cleanup() {
        for (auto& model : m_models) model.second->cleanup();

        for (auto& mesh : m_meshes) mesh.second.cleanup();

        for (auto& texture : m_textures) texture.second.cleanup(m_device->m_logicalDevice);

        for (auto& shader : m_shaders) shader->cleanup(m_device->m_logicalDevice);

        m_device->m_logicalDevice.destroy(m_skinsDescriptorSetLayout);
        m_device->m_logicalDevice.destroy(m_skinSDescriptorPool);
        m_device->m_logicalDevice.destroy(m_imagesDescriptorSetLayout);
    }

    void ResourceManager::createTexture(const std::string &fileName, const std::string& name) {
        if (m_textures.find(engine::tools::hashString(name)) != m_textures.end()) {
            return ;
        }

        int width, height;
        vk::DeviceSize imageSize;
        stbi_uc* pixels = engine::tools::loadTextureFile(fileName, &width, &height, &imageSize);
        engine::Buffer stagingBuffer;

        stagingBuffer = m_device->createBuffer(vk::BufferUsageFlagBits::eTransferSrc,
                               vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                               imageSize);

        stagingBuffer.map(imageSize);
        stagingBuffer.copyTo(pixels, imageSize);
        stagingBuffer.unmap();

        stbi_image_free(pixels);

        vk::Extent2D size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
        auto mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height))));

        engine::Texture texture(m_device->m_logicalDevice, size, vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal,
                              vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                                mipLevels);

        vk::MemoryRequirements memoryRequirements = m_device->m_logicalDevice.getImageMemoryRequirements(texture.getTextureImage().getImage());

        auto memType = m_device->getMemoryType(memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

        texture.bind(m_device->m_logicalDevice, memType, memoryRequirements.size);

        m_device->transitionImageLayout(texture.getTextureImage().getImage(), vk::Format::eR8G8B8Srgb, m_graphicsQueue,
                                        vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, mipLevels);

        m_device->copyBufferToImage(stagingBuffer.m_buffer, texture.getTextureImage().getImage(), m_graphicsQueue,
                                    size);

        generateMipmaps(texture, vk::Format::eR8G8B8A8Srgb, size, mipLevels);

        stagingBuffer.destroy();

        texture.createDescriptor(m_device->m_logicalDevice, m_imagesDescriptorPool, m_imagesDescriptorSetLayout);

        m_textures[engine::tools::hashString(name)] = texture;
    }

    engine::Texture &ResourceManager::getTexture(uint64_t id) {
        return m_textures[id];
    }

    vk::DescriptorSetLayout &ResourceManager::getTextureDescriptorSetLayout() {
        return m_imagesDescriptorSetLayout;
    }

    void ResourceManager::recreateResources() {
       createDescriptorPool();

       for (auto& texture : m_textures) {
           texture.second.createDescriptor(m_device->m_logicalDevice, m_imagesDescriptorPool, m_imagesDescriptorSetLayout);
       }
    }

    void ResourceManager::cleanupResources() {
        vkDestroyDescriptorPool(m_device->m_logicalDevice, m_imagesDescriptorPool, nullptr);
    }

    void ResourceManager::generateMipmaps(const engine::Texture& texture, vk::Format format, vk::Extent2D size, uint32_t mipLevels) {
        vk::Image textureImage = texture.getTextureImage().getImage();
        // Check if image format supports linear blitting
        vk::FormatProperties formatProperties = m_device->m_physicalDevice.getFormatProperties(format);

        if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
            engine::throw_ex("texture image format does not support linear blitting");
        }

        vk::CommandBuffer commandBuffer = m_device->createCommandBuffer(vk::CommandBufferLevel::ePrimary, true);

        vk::ImageMemoryBarrier barrier{
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = textureImage,
            .subresourceRange = {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
            }
        };

        auto mipWidth = static_cast<int32_t>(size.width);
        auto mipHeight = static_cast<int32_t>(size.height);

        for (uint32_t i = 1; i < mipLevels; ++i) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

            commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {},
                                          0, nullptr, 0, nullptr, 1, &barrier);

            vk::ImageBlit blit{
                .srcSubresource = {
                        .aspectMask = vk::ImageAspectFlagBits::eColor,
                        .mipLevel = i - 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1
                },
                .srcOffsets = {std::array<vk::Offset3D, 2>({ { {0, 0, 0}, {mipHeight, mipWidth, 1} } })},
                .dstSubresource = {
                        .aspectMask = vk::ImageAspectFlagBits::eColor,
                        .mipLevel = i,
                        .baseArrayLayer = 0,
                        .layerCount = 1
                },
                .dstOffsets = {std::array<vk::Offset3D, 2>({ { {0, 0, 0}, {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1} } })}
            };

            commandBuffer.blitImage(textureImage, vk::ImageLayout::eTransferSrcOptimal, textureImage, vk::ImageLayout::eTransferDstOptimal,
                                    1, &blit, vk::Filter::eLinear);

            barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
            barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {},
                                          0, nullptr, 0, nullptr, 1, &barrier);

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }

        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {},
                                      0, nullptr, 0, nullptr, 1, &barrier);

        m_device->flushCommandBuffer(commandBuffer, m_graphicsQueue);
    }

    void ResourceManager::createDescriptorPool() {
        vk::DescriptorPoolSize samplerPoolSizer{
            .type = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = MAX_OBJECTS
        };

        vk::DescriptorPoolCreateInfo samplerPoolCreateInfo{
            .maxSets = MAX_OBJECTS,
            .poolSizeCount = 1,
            .pPoolSizes = &samplerPoolSizer
        };

        m_imagesDescriptorPool = m_device->m_logicalDevice.createDescriptorPool(samplerPoolCreateInfo);
    }

    void ResourceManager::createDescriptorSetLayout() {
        vk::DescriptorSetLayoutBinding samplerLayoutBinding{
            .binding = 0,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eFragment,
            .pImmutableSamplers = nullptr
        };

        vk::DescriptorSetLayoutCreateInfo samplerLayoutInfo{
            .bindingCount = 1,
            .pBindings = &samplerLayoutBinding,
        };

        m_imagesDescriptorSetLayout = m_device->m_logicalDevice.createDescriptorSetLayout(samplerLayoutInfo);
    }

    uint64_t ResourceManager::createModel(const std::string &uri, const std::string& name) {
        uint64_t modelName = engine::tools::hashString(name);

        if (m_models.find(modelName) != m_models.end()) return modelName;

        json modelFile;
        {
            std::ifstream file("../data/models/" + uri);
            file >> modelFile;
            file.close();
        }

        tinygltf::Model inputModel;
        tinygltf::TinyGLTF loader;
        std::string error, warning;

        bool fileLoaded = loader.LoadASCIIFromFile(&inputModel, &error, &warning, MODELS_DIR + modelFile["name"].get<std::string>() + ".gltf");

        if (fileLoaded) {
            m_models[modelName] = std::make_shared<engine::Model>(name, inputModel.nodes.size());

            for (auto& image : inputModel.images) createTexture(image.uri, image.name);

            for (auto& nodeID : inputModel.scenes[0].nodes) m_models[modelName]->loadNode(inputModel.nodes[nodeID], inputModel, nodeID);

            if (!Application::m_editor) m_models[modelName]->loadSkins(inputModel, m_device, m_graphicsQueue);

            if (!modelFile["animations"].empty() && !Application::m_editor)
                m_models[modelName]->m_animations.ide = loadAnimation(modelFile["animations"]["ide"].get<std::string>(), name + "_ide");

            return modelName;
        } else {
            fmt::print(stderr, "[Model] error: {} \n", error);

            return 0;
        }
    }

    std::shared_ptr<engine::Model> ResourceManager::getModel(uint64_t id) {
        return m_models[id];
    }

    engine::Mesh &ResourceManager::getMesh(uint64_t id) {
        return m_meshes[id];
    }

    uint64_t ResourceManager::loadMesh(const std::string& name, const tinygltf::Mesh &mesh, const tinygltf::Model &model, uint64_t texturesID) {
        uint64_t meshID = engine::tools::hashString(name);

        if (m_meshes.find(meshID) != m_meshes.end()) {
            return meshID;
        }

        std::vector<engine::Vertex> vertices;
        std::vector<uint32_t> indices;

        for (auto primitive : mesh.primitives) {
            uint32_t indexCount = 0;

            // Vertices
            {
                const float* positionBuffer = nullptr;
                const float* normalsBuffer = nullptr;
                const float* texCoordsBuffer = nullptr;
                const uint16_t* jointIndicesBuffer = nullptr;
                const float* jointWeightsBuffer = nullptr;
                size_t vertexCount = 0;

                if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("POSITION")->second];
                    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
                    positionBuffer = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                    vertexCount = accessor.count;
                }

                if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("NORMAL")->second];
                    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
                    normalsBuffer = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }

                if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
                    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
                    texCoordsBuffer = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }

                if (primitive.attributes.find("JOINTS_0") != primitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("JOINTS_0")->second];
                    const tinygltf::BufferView &view = model.bufferViews[accessor.bufferView];
                    jointIndicesBuffer = reinterpret_cast<const uint16_t *>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }

                if (primitive.attributes.find("WEIGHTS_0") != primitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("WEIGHTS_0")->second];
                    const tinygltf::BufferView &view = model.bufferViews[accessor.bufferView];
                    jointWeightsBuffer = reinterpret_cast<const float *>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }

                bool hasSkin = (jointIndicesBuffer && jointWeightsBuffer);

                for (size_t v = 0; v < vertexCount; ++v) {
                    engine::Vertex vertex{};
                    vertex.position = glm::make_vec3(&positionBuffer[v * 3]);
                    vertex.normal = glm::normalize(glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));
                    vertex.texCoord = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec2(0.0f);
                    vertex.color = glm::vec3(1.0f);
                    vertex.jointIndices = hasSkin ? glm::vec4(glm::make_vec4(&jointIndicesBuffer[v * 4])) : glm::vec4(0.0f);
                    vertex.jointWeights = hasSkin ? glm::make_vec4(&jointWeightsBuffer[v * 4]) : glm::vec4(0.0f);

                    vertices.push_back(vertex);
                }
            }

            // Indices
            {
                const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                indexCount += static_cast<uint32_t>(accessor.count);

                switch (accessor.componentType) {
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
                        auto* buf = new uint32_t[accessor.count];

                        std::memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint32_t));

                        for (size_t index = 0; index < accessor.count; ++index) {
                            indices.push_back(buf[index]);
                        }

                        delete[] buf;
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                        auto* buf = new uint16_t[accessor.count];

                        std::memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint16_t));

                        for (size_t index = 0; index < accessor.count; index++) indices.push_back(buf[index]);

                        delete[] buf;
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                        auto* buf = new uint8_t[accessor.count];

                        std::memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint8_t));

                        for (size_t index = 0; index < accessor.count; index++) indices.push_back(buf[index]);

                        delete[] buf;
                        break;
                    }
                }
            }
        }

        // TODO: Check validation layer for use CommandPool with different queue family index
        m_meshes[meshID] = engine::Mesh(vertices, indices, m_device->m_logicalDevice.getQueue(m_device->m_queueFamilyIndices.transfer, 0), texturesID, m_device);

        return meshID;
    }

    std::shared_ptr<engine::Shader> ResourceManager::createShader(const std::string &vert, const std::string &frag, const std::vector<vk::PushConstantRange>& pushConstants, bool vertexInfo) {
        m_shaders.emplace_back(std::make_shared<Shader>(vert, frag, m_device->m_logicalDevice, pushConstants, vertexInfo));

        return m_shaders.back();
    }

    uint64_t ResourceManager::loadAnimation(const std::string& uri, const std::string& name) {
        uint64_t animationName = std::hash<std::string>{}(name);

        if (m_animations.find(animationName) != m_animations.end()) return animationName;

        tinygltf::Model inputModel;
        tinygltf::TinyGLTF loader;
        std::string error, warning;

        if (loader.LoadASCIIFromFile(&inputModel, &error, &warning, ANIMATIONS_DIR + uri + ".gltf")) {
            tinygltf::Animation gltfAnimation = inputModel.animations[0];
            m_animations[animationName] = Animation();
            Animation& animation = m_animations[animationName];
            animation.m_name = gltfAnimation.name;

            animation.m_samplers.resize(gltfAnimation.samplers.size());
            for (int i = 0; i < gltfAnimation.samplers.size(); ++i) {
                tinygltf::AnimationSampler& glTFSampler = gltfAnimation.samplers[i];
                Animation::Sampler& dstSampler = animation.m_samplers[i];
                dstSampler.interpolation = glTFSampler.interpolation;

                // Read sampler keyframe input time values
                {
                    const tinygltf::Accessor&  accessor = inputModel.accessors[glTFSampler.input];
                    const tinygltf::BufferView &bufferView = inputModel.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer& buffer = inputModel.buffers[bufferView.buffer];
                    const void *dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
                    const auto *buf = static_cast<const float *>(dataPtr);

                    dstSampler.inputs.resize(accessor.count);
                    for (size_t index = 0; index < accessor.count; ++index)
                        dstSampler.inputs[index] = buf[index];

                    for (auto input : animation.m_samplers[i].inputs) {
                        if (input < animation.m_start) animation.m_start = input;

                        if (input > animation.m_end) animation.m_end = input;
                    }
                }

                // Read sampler keyframe output translate/rotate/scale values
                {
                    const tinygltf::Accessor& accessor = inputModel.accessors[glTFSampler.output];
                    const tinygltf::BufferView& bufferView = inputModel.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer& buffer = inputModel.buffers[bufferView.buffer];
                    const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];

                    switch (accessor.type) {
                        case TINYGLTF_TYPE_VEC3: {
                            const auto *buf = static_cast<const glm::vec3 *>(dataPtr);

                            for (size_t index = 0; index < accessor.count; index++) dstSampler.outputs.emplace_back(buf[index], 0.0f);

                            break;
                        }
                        case TINYGLTF_TYPE_VEC4: {
                            const auto *buf = static_cast<const glm::vec4 *>(dataPtr);

                            for (size_t index = 0; index < accessor.count; index++) dstSampler.outputs.push_back(buf[index]);

                            break;
                        }
                        default: {
                            fmt::print("Unknown type\n");
                            break;
                        }
                    }
                }
            }

            // Channels
            animation.m_channels.resize(gltfAnimation.channels.size());
            for (int i = 0; i < gltfAnimation.channels.size(); ++i) {
                tinygltf::AnimationChannel gltfChannel = gltfAnimation.channels[i];
                Animation::Channel& dstChannel = animation.m_channels[i];
                dstChannel.path = gltfChannel.target_path;
                dstChannel.samplerIndex = gltfChannel.sampler;
                dstChannel.nodeID = gltfChannel.target_node;
            }
        } else {
            return 0;
        }

        return animationName;
    }

    Animation &ResourceManager::getAnimation(uint64_t name) {
        return m_animations[name];
    }

    void ResourceManager::createSkinsDescriptors(const std::vector<vk::DescriptorPoolSize>& sizes, uint32_t maxSize) {
        vk::DescriptorPoolCreateInfo poolCreateInfo{
            .maxSets = maxSize,
            .poolSizeCount = static_cast<uint32_t>(sizes.size()),
            .pPoolSizes = sizes.data()
        };
        
        m_skinSDescriptorPool = m_device->m_logicalDevice.createDescriptorPool(poolCreateInfo);

        vk::DescriptorSetLayoutBinding layoutBinding{
            .binding = 0,
            .descriptorType = vk::DescriptorType::eStorageBuffer,
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eVertex,
            .pImmutableSamplers = nullptr
        };

        vk::DescriptorSetLayoutCreateInfo layoutCreateInfo{
            .bindingCount = 1,
            .pBindings = &layoutBinding
        };

        m_skinsDescriptorSetLayout = m_device->m_logicalDevice.createDescriptorSetLayout(layoutCreateInfo);
    }

    void ResourceManager::createSkinsDescriptorSets() {
        for (auto& [id, model] : m_models) {
            for (auto& node : model->getNodes()) {
                if (node.skin > -1) {
                    Model::Skin& skin = model->getSkin(node.skin);
                    vk::DescriptorSetAllocateInfo allocateInfo{
                        .descriptorPool = m_skinSDescriptorPool,
                        .descriptorSetCount = 1,
                        .pSetLayouts = &m_skinsDescriptorSetLayout
                    };

                    skin.descriptorSet = m_device->m_logicalDevice.allocateDescriptorSets(allocateInfo).front();
                    skin.ssbo.setupDescriptor(sizeof(glm::mat4) * skin.joints.size());

                    vk::WriteDescriptorSet writeDescriptorSet{
                        .dstSet = skin.descriptorSet,
                        .dstBinding = 0,
                        .descriptorCount = 1,
                        .descriptorType = vk::DescriptorType::eStorageBuffer,
                        .pBufferInfo= &skin.ssbo.m_descriptor
                    };

                    m_device->m_logicalDevice.updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
                }
            }
        }
    }

    uint32_t ResourceManager::getSkinsCount() {
        uint32_t skinsCount = 0;

        for (auto& [id, model] : m_models) {
            skinsCount += static_cast<uint32_t>(model->getSkinsCount());
        }

        return skinsCount;
    }

    vk::DescriptorSetLayout ResourceManager::getSkinsDescriptorSetLayout() {
        return m_skinsDescriptorSetLayout;
    }

} // namespace core