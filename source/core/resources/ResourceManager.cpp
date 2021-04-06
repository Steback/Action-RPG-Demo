#include "ResourceManager.hpp"

#include <utility>

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"
#include "fmt/format.h"
#include "glm/detail/type_quat.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Shader.hpp"


namespace core {

    ResourceManager::ResourceManager(std::shared_ptr<vkc::Device> device, vk::Queue graphicsQueue)
            : m_device(std::move(device)), m_graphicsQueue(graphicsQueue) {
        createDescriptorSetLayout();
        createDescriptorPool();
    }

    ResourceManager::~ResourceManager() = default;

    void ResourceManager::cleanup() {
        for (auto& mesh : m_meshes) mesh.second.cleanup();

        for (auto& texture : m_textures) texture.second.cleanup(m_device->m_logicalDevice);

        for (auto& shader : m_shaders) shader->cleanup(m_device->m_logicalDevice);

        vkDestroyDescriptorSetLayout(m_device->m_logicalDevice, m_descriptorSetLayout, nullptr);
    }

    void ResourceManager::createTexture(const std::string &fileName, const std::string& name) {
        int width, height;
        vk::DeviceSize imageSize;
        stbi_uc* pixels = core::tools::loadTextureFile(fileName, &width, &height, &imageSize);
        vkc::Buffer stagingBuffer;

        stagingBuffer = m_device->createBuffer(vk::BufferUsageFlagBits::eTransferSrc,
                               vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                               imageSize);

        stagingBuffer.map(imageSize);
        stagingBuffer.copyTo(pixels, imageSize);
        stagingBuffer.unmap();

        stbi_image_free(pixels);

        vk::Extent2D size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
        auto mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height))));

        core::Texture texture(m_device->m_logicalDevice, size, vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal,
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

        texture.createDescriptor(m_device->m_logicalDevice, m_descriptorPool, m_descriptorSetLayout);

        m_textures[core::tools::hashString(name)] = texture;
    }

    core::Texture &ResourceManager::getTexture(uint64_t id) {
        return m_textures[id];
    }

    vk::DescriptorSetLayout &ResourceManager::getTextureDescriptorSetLayout() {
        return m_descriptorSetLayout;
    }

    void ResourceManager::recreateResources() {
       createDescriptorPool();

       for (auto& texture : m_textures) {
           texture.second.createDescriptor(m_device->m_logicalDevice, m_descriptorPool, m_descriptorSetLayout);
       }
    }

    void ResourceManager::cleanupResources() {
        vkDestroyDescriptorPool(m_device->m_logicalDevice, m_descriptorPool, nullptr);
    }

    void ResourceManager::generateMipmaps(const core::Texture& texture, vk::Format format, vk::Extent2D size, uint32_t mipLevels) {
        vk::Image textureImage = texture.getTextureImage().getImage();
        // Check if image format supports linear blitting
        vk::FormatProperties formatProperties = m_device->m_physicalDevice.getFormatProperties(format);

        if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
            core::throw_ex("texture image format does not support linear blitting");
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

        m_descriptorPool = m_device->m_logicalDevice.createDescriptorPool(samplerPoolCreateInfo);
    }

    void ResourceManager::createDescriptorSetLayout() {
        vk::DescriptorSetLayoutBinding samplerLayoutBinding{
            .binding = 1,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eFragment,
            .pImmutableSamplers = nullptr
        };

        vk::DescriptorSetLayoutCreateInfo samplerLayoutInfo{
            .bindingCount = 1,
            .pBindings = &samplerLayoutBinding
        };

        m_descriptorSetLayout = m_device->m_logicalDevice.createDescriptorSetLayout(samplerLayoutInfo);
    }

    uint64_t ResourceManager::createModel(const std::string &uri, const std::string& name) {
        tinygltf::Model inputModel;
        tinygltf::TinyGLTF loader;
        std::string error, warning;

        bool fileLoaded = loader.LoadASCIIFromFile(&inputModel, &error, &warning, MODELS_DIR + uri);

        if (fileLoaded) {
            uint64_t modelName = core::tools::hashString(name);
            m_models[modelName] = std::make_shared<core::Model>(name);

            for (auto& image : inputModel.images) createTexture(image.uri, image.name);

            for (auto& nodeID : inputModel.scenes[0].nodes) m_models[modelName]->loadNode(inputModel.nodes[nodeID], inputModel);

            return modelName;

        } else {
            fmt::print(stderr, "[Model] error: {} \n", error);

            return 0;
        }
    }

    std::shared_ptr<core::Model> ResourceManager::getModel(uint64_t id) {
        return m_models[id];
    }

    core::Mesh &ResourceManager::getMesh(uint64_t id) {
        return m_meshes[id];
    }

    uint64_t ResourceManager::loadMesh(const std::string& name, const tinygltf::Mesh &mesh, const tinygltf::Model &model, uint64_t texturesID) {
        std::vector<core::Vertex> vertices;
        std::vector<uint32_t> indices;

        for (auto primitive : mesh.primitives) {
            uint32_t indexCount = 0;

            // Vertices
            {
                const float* positionBuffer = nullptr;
                const float* normalsBuffer = nullptr;
                const float* texCoordsBuffer = nullptr;
                size_t vertexCount = 0;

                if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("POSITION")->second];
                    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];

                    positionBuffer = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                    vertexCount = accessor.count;
                }

                if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
                    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];

                    texCoordsBuffer = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }

                for (size_t v = 0; v < vertexCount; ++v) {
                    core::Vertex vertex{};
                    vertex.position = glm::make_vec3(&positionBuffer[v * 3]);
                    vertex.texCoord = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec2(0.0f);
                    vertex.color = glm::vec3(1.0f);

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

        uint64_t meshID = core::tools::hashString(name);
        m_meshes[meshID] = core::Mesh(vertices, indices, m_graphicsQueue, texturesID, m_device);

        return meshID;
    }

    std::shared_ptr<core::Shader> ResourceManager::createShader(const std::string &vert, const std::string &frag, bool vertexInfo) {
        m_shaders.push_back(std::make_shared<core::Shader>(vert, frag, m_device->m_logicalDevice, vertexInfo));

        return m_shaders.back();
    }

} // namespace core
