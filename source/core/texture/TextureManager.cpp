#include "TextureManager.hpp"

#include "../renderer/Tools.hpp"


namespace core {

    TextureManager::TextureManager(vk::Device *device, VkQueue graphicsQueue)
            : m_device(device), m_graphicsQueue(graphicsQueue) {
        createDescriptorSetLayout();
        createDescriptorPool();
        createTextureSampler();
    }

    TextureManager::~TextureManager() = default;

    void TextureManager::cleanup() {
        for (auto& texture : textures) {
            texture.cleanup(m_device->m_logicalDevice);
        }

        vkDestroySampler(m_device->m_logicalDevice, m_textureSampler, nullptr);
        vkDestroyDescriptorSetLayout(m_device->m_logicalDevice, m_descriptorSetLayout, nullptr);
    }

    uint TextureManager::createTexture(const std::string &fileName) {
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

        auto u32Width = static_cast<uint32_t>(width);
        auto u32Height = static_cast<uint32_t>(height);

        core::Texture texture(m_device->m_logicalDevice, u32Width, u32Height,
                              VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                              VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

        VkMemoryRequirements memoryRequirements{};
        vkGetImageMemoryRequirements(m_device->m_logicalDevice, texture.getTextureImage().getImage(), &memoryRequirements);

        auto memType = m_device->getMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        texture.bind(m_device->m_logicalDevice, memType, memoryRequirements.size);

        m_device->transitionImageLayout(texture.getTextureImage().getImage(), VK_FORMAT_R8G8B8A8_SRGB, m_graphicsQueue,
                                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        m_device->copyBufferToImage(stagingBuffer.m_buffer, texture.getTextureImage().getImage(), m_graphicsQueue,
                                    u32Width, u32Height);

        m_device->transitionImageLayout(texture.getTextureImage().getImage(), VK_FORMAT_R8G8B8A8_SRGB, m_graphicsQueue,
                                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        stagingBuffer.destroy();

        texture.createDescriptor(m_device->m_logicalDevice, m_descriptorPool, m_textureSampler, m_descriptorSetLayout);

        textures.push_back(texture);

        return static_cast<uint>(textures.size()) - 1;
    }

    core::Texture &TextureManager::getTexture(size_t index) {
        return textures[index];
    }

    VkDescriptorSet TextureManager::getTextureDescriptorSet(size_t index) {
        return textures[index].getDescriptorSet();
    }

    void TextureManager::recreateResources() {
       createDescriptorPool();

       for (auto& texture : textures) {
           texture.createDescriptor(m_device->m_logicalDevice, m_descriptorPool, m_textureSampler, m_descriptorSetLayout);
       }
    }

    void TextureManager::cleanupResources() {
        vkDestroyDescriptorPool(m_device->m_logicalDevice, m_descriptorPool, nullptr);
    }

    void TextureManager::createDescriptorPool() {
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

    void TextureManager::createDescriptorSetLayout() {
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

    void TextureManager::createTextureSampler() {
        VkSamplerCreateInfo samplerInfo = vk::initializers::samplerCreateInfo();
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        vk::tools::validation(vkCreateSampler(m_device->m_logicalDevice, &samplerInfo, nullptr, &m_textureSampler),
                              "Failed to create texture sampler");
    }

    VkDescriptorSetLayout TextureManager::getDescriptorSetLayout() {
        return m_descriptorSetLayout;
    }

} // namespace core
