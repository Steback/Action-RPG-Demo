#include "Texture.hpp"

#include "../Utilities.hpp"
#include "../renderer/Tools.hpp"


namespace core {

    Texture::Texture() = default;

    Texture::Texture(VkDevice logicalDevice, VkExtent2D size, VkFormat format, VkImageTiling tiling,
                     VkImageUsageFlags usageFlags, uint32_t mipLevels) {

        VkImageCreateInfo createInfo = vk::initializers::imageCreateInfo();
        createInfo.imageType = VK_IMAGE_TYPE_2D; // Type of image (1D, 2D or 3D)
        createInfo.extent.width = size.width; // Width of Image extent
        createInfo.extent.height = size.height; // Height of Image extent
        createInfo.extent.depth = 1; // Depth of image (just 1, no 3D aspect)
        createInfo.mipLevels = mipLevels; // Number of mipmap levels
        createInfo.arrayLayers = 1; // Number of levels in image array
        createInfo.format = format; // Format type of image
        createInfo.tiling = tiling; // How image data should be "tiled" (arranged for optimal reading)
        createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Layout of image data on creation
        createInfo.usage = usageFlags; // Bit flags defining what image will be use for
        createInfo.samples = VK_SAMPLE_COUNT_1_BIT; // Number of samples for multi-sampling
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        m_image = vk::Image(logicalDevice, createInfo);

        createTextureSampler(logicalDevice, mipLevels);
    }

    Texture::~Texture() = default;

    void Texture::bind(VkDevice logicalDevice, uint32_t memoryTypeIndex, VkDeviceSize size) {
        m_image.bind(logicalDevice, memoryTypeIndex, size, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    // TODO: Check for move descriptor set in resize window
    void Texture::createDescriptor(VkDevice logicalDevice, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout) {
        VkDescriptorSetAllocateInfo setAllocateInfo{};
        setAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        setAllocateInfo.descriptorPool = descriptorPool;
        setAllocateInfo.descriptorSetCount = 1;
        setAllocateInfo.pSetLayouts = &descriptorSetLayout;

        VkResult result = vkAllocateDescriptorSets(logicalDevice, &setAllocateInfo, &m_descriptorSet);

        if (result != VK_SUCCESS) throw std::runtime_error("Failed to allocate Texture Descriptor Sets");

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = m_image.getView();
        imageInfo.sampler = m_sampler;

        VkWriteDescriptorSet writeDescriptorSet = vk::initializers::writeDescriptorSet();
        writeDescriptorSet.dstSet = m_descriptorSet;
        writeDescriptorSet.dstBinding = 1;
        writeDescriptorSet.dstArrayElement = 0;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(logicalDevice, 1, &writeDescriptorSet, 0, nullptr);
    }

    void Texture::cleanup(VkDevice logicalDevice) {
        vkDestroySampler(logicalDevice, m_sampler, nullptr);
        m_image.cleanup(logicalDevice);
    }

    uint32_t Texture::getWidth() const {
        return m_image.getWidth();
    }

    uint32_t Texture::getHeight() const {
        return m_image.getHeight();
    }

    vk::Image Texture::getTextureImage() const {
        return m_image;
    }

    VkImageView Texture::getImageView() const {
        return m_image.getView();
    }

    uint32_t Texture::getMipLevel() const {
        return m_image.getMipLevel();
    }

    VkDescriptorSet Texture::getDescriptorSet() const {
        return m_descriptorSet;
    }

    void Texture::createTextureSampler(VkDevice logicalDevice, uint32_t mipLevels) {
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
        samplerInfo.maxLod = static_cast<float>(mipLevels);

        vk::tools::validation(vkCreateSampler(logicalDevice, &samplerInfo, nullptr, &m_sampler),
                              "Failed to create texture sampler");
    }

} // namespace core
