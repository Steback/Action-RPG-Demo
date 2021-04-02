#include "ResourceManager.hpp"

#include <utility>

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"
#include "fmt/format.h"
#include "glm/detail/type_quat.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "../renderer/Tools.hpp"

namespace core {

    ResourceManager::ResourceManager(std::shared_ptr<vkc::Device> device, VkQueue graphicsQueue)
            : m_device(std::move(device)), m_graphicsQueue(graphicsQueue) {
        createDescriptorSetLayout();
        createDescriptorPool();
    }

    ResourceManager::~ResourceManager() = default;

    void ResourceManager::cleanup() {
        for (auto& mesh : m_meshes) mesh.second.cleanup();

        for (auto& texture : m_textures) texture.second.cleanup(m_device->m_logicalDevice);

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

        core::Texture texture(m_device->m_logicalDevice, size, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                              VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                              mipLevels);

        VkMemoryRequirements memoryRequirements{};
        vkGetImageMemoryRequirements(m_device->m_logicalDevice, texture.getTextureImage().getImage(), &memoryRequirements);

        auto memType = m_device->getMemoryType(memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

        texture.bind(m_device->m_logicalDevice, memType, memoryRequirements.size);

        m_device->transitionImageLayout(texture.getTextureImage().getImage(), vk::Format::eR8G8B8Srgb, m_graphicsQueue,
                                        vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, mipLevels);

        m_device->copyBufferToImage(stagingBuffer.m_buffer, texture.getTextureImage().getImage(), m_graphicsQueue,
                                    size);

        generateMipmaps(texture, VK_FORMAT_R8G8B8A8_SRGB, size, mipLevels);

        stagingBuffer.destroy();

        texture.createDescriptor(m_device->m_logicalDevice, m_descriptorPool, m_descriptorSetLayout);

        m_textures[core::tools::hashString(name)] = texture;
    }

    core::Texture &ResourceManager::getTexture(uint64_t id) {
        return m_textures[id];
    }

    VkDescriptorSetLayout &ResourceManager::getTextureDescriptorSetLayout() {
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

    void ResourceManager::generateMipmaps(const core::Texture& texture, VkFormat format, VkExtent2D size, uint32_t mipLevels) {
        // Check if image format supports linear blitting
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(m_device->m_physicalDevice, format, &formatProperties);

        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
            core::throw_ex("texture image format does not support linear blitting");
        }

        vk::CommandBuffer commandBuffer = m_device->createCommandBuffer(vk::CommandBufferLevel::ePrimary, true);

        VkImageMemoryBarrier barrier = vkc::initializers::imageMemoryBarrier();
        barrier.image = texture.getTextureImage().getImage();
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        auto mipWidth = static_cast<int32_t>(size.width);
        auto mipHeight = static_cast<int32_t>(size.height);

        for (uint32_t i = 1; i < mipLevels; ++i) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                                 0, nullptr,
                                 0, nullptr,
                                 1, &barrier);

            VkImageBlit blit{};
            blit.srcOffsets[0] = { 0, 0, 0 };
            blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;

            vkCmdBlitImage(commandBuffer,
                           texture.getTextureImage().getImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           texture.getTextureImage().getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1, &blit,
                           VK_FILTER_LINEAR);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                                 0, nullptr,
                                 0, nullptr,
                                 1, &barrier);

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }

        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);

        m_device->flushCommandBuffer(commandBuffer, m_graphicsQueue);
    }

    void ResourceManager::createDescriptorPool() {
        VkDescriptorPoolSize samplerPoolSizer{};
        samplerPoolSizer.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerPoolSizer.descriptorCount = MAX_OBJECTS;

        VkDescriptorPoolCreateInfo samplerPoolCreateInfo{};
        samplerPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        samplerPoolCreateInfo.maxSets = MAX_OBJECTS;
        samplerPoolCreateInfo.poolSizeCount = 1;
        samplerPoolCreateInfo.pPoolSizes = &samplerPoolSizer;

        VK_CHECK_RESULT(vkCreateDescriptorPool(m_device->m_logicalDevice, &samplerPoolCreateInfo, nullptr, &m_descriptorPool))
    }

    void ResourceManager::createDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo samplerLayoutInfo = vkc::initializers::descriptorSetLayoutCreateInfo();
        samplerLayoutInfo.bindingCount = 1;
        samplerLayoutInfo.pBindings = &samplerLayoutBinding;

        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_device->m_logicalDevice, &samplerLayoutInfo, nullptr, &m_descriptorSetLayout))
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

} // namespace core
