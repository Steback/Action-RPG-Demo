#include "ResourceManager.hpp"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"
#include "fmt/format.h"
#include "glm/detail/type_quat.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "../renderer/Tools.hpp"

namespace core {

    ResourceManager::ResourceManager(vk::Device *device, VkQueue graphicsQueue)
            : m_device(device), m_graphicsQueue(graphicsQueue) {
        createDescriptorSetLayout();
        createDescriptorPool();
    }

    ResourceManager::~ResourceManager() = default;

    void ResourceManager::cleanup() {
        for (auto& model : m_models) model.second.cleanup();

        for (auto& mesh : m_meshes) mesh.second.cleanup();

        for (auto& texture : m_textures) texture.second.cleanup(m_device->m_logicalDevice);

        vkDestroyDescriptorSetLayout(m_device->m_logicalDevice, m_descriptorSetLayout, nullptr);
    }

    void ResourceManager::createTexture(const std::string &fileName, const std::string& name) {
        int width, height;
        VkDeviceSize imageSize;
        stbi_uc* pixels = core::tools::loadTextureFile(fileName, &width, &height, &imageSize);
        vk::Buffer stagingBuffer;

        m_device->createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                               &stagingBuffer, imageSize);

        stagingBuffer.map(imageSize);
        stagingBuffer.copyTo(pixels, imageSize);
        stagingBuffer.unmap();

        stbi_image_free(pixels);

        VkExtent2D size = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
        auto mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height))));

        core::Texture texture(m_device->m_logicalDevice, size, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                              VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                              mipLevels);

        VkMemoryRequirements memoryRequirements{};
        vkGetImageMemoryRequirements(m_device->m_logicalDevice, texture.getTextureImage().getImage(), &memoryRequirements);

        auto memType = m_device->getMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        texture.bind(m_device->m_logicalDevice, memType, memoryRequirements.size);

        m_device->transitionImageLayout(texture.getTextureImage().getImage(), VK_FORMAT_R8G8B8A8_SRGB, m_graphicsQueue,
                                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels );

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

        VkCommandBuffer commandBuffer = m_device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

        VkImageMemoryBarrier barrier = vk::initializers::imageMemoryBarrier();
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

        m_device->flushCommandBuffer(commandBuffer, m_graphicsQueue, true);
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

        vk::tools::validation(vkCreateDescriptorPool(m_device->m_logicalDevice, &samplerPoolCreateInfo, nullptr, &m_descriptorPool),
                              "Failed to create a Descriptor Pool");
    }

    void ResourceManager::createDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo samplerLayoutInfo = vk::initializers::descriptorSetLayoutCreateInfo();
        samplerLayoutInfo.bindingCount = 1;
        samplerLayoutInfo.pBindings = &samplerLayoutBinding;

        vk::tools::validation(vkCreateDescriptorSetLayout(m_device->m_logicalDevice, &samplerLayoutInfo, nullptr, &m_descriptorSetLayout),
                              "Failed to create descriptor set layout");
    }

    // TODO: Check create model
    void ResourceManager::createModel(const std::string &uri, const std::string& name) {
        tinygltf::Model model;
        tinygltf::TinyGLTF loader;
        std::string error, warning;

        bool fileLoaded = loader.LoadASCIIFromFile(&model, &error, &warning, MODELS_DIR + uri);

        if (fileLoaded) {
            std::vector<core::Model::Node> nodes;
            std::vector<uint32_t> indices;
            std::vector<core::Vertex> vertices;
            uint meshNodeID;
            uint64_t meshID;

            for (auto& image : model.images) createTexture(image.uri, image.name);

            for (auto& nodeID : model.scenes[0].nodes) core::Model::loadNode(model.nodes[nodeID], model, meshNodeID, nodes);

            // TODO: Update textures names load for X amount of textures
            for (auto& mesh : model.meshes) {
                std::string textureName = (model.images.empty() ? "plain" : model.images[0].name);
                meshID = core::tools::hashString(nodes[meshNodeID].name);

                m_meshes[meshID] = core::Model::loadMesh(m_device, m_graphicsQueue, mesh, model, core::tools::hashString(textureName));
            }

            m_models[core::tools::hashString(name)] = core::Model(meshID, nodes, meshNodeID);
        } else {
            fmt::print(stderr, "[Model] error: {} \n", error);
        }
    }

    core::Model& ResourceManager::getModel(uint64_t id) {
        return m_models[id];
    }

    core::Mesh &ResourceManager::getMesh(uint64_t id) {
        return m_meshes[id];
    }

} // namespace core
