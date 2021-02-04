#include "Texture.hpp"

#include "../Utilities.hpp"


namespace core {

    Texture::Texture() = default;

    Texture::Texture(VkDevice logicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                     VkImageUsageFlags usageFlags) {
        VkImageCreateInfo createInfo = vk::initializers::imageCreateInfo();
        createInfo.imageType = VK_IMAGE_TYPE_2D; // Type of image (1D, 2D or 3D)
        createInfo.extent.width = width; // Width of Image extent
        createInfo.extent.height = height; // Height of Image extent
        createInfo.extent.depth = 1; // Depth of image (just 1, no 3D aspect)
        createInfo.mipLevels = 1; // Number of mipmap levels
        createInfo.arrayLayers = 1; // Number of levels in image array
        createInfo.format = format; // Format type of image
        createInfo.tiling = tiling; // How image data should be "tiled" (arranged for optimal reading)
        createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Layout of image data on creation
        createInfo.usage = usageFlags; // Bit flags defining what image will be use for
        createInfo.samples = VK_SAMPLE_COUNT_1_BIT; // Number of samples for multi-sampling
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        m_image = vk::Image(logicalDevice, createInfo);
    }

    Texture::~Texture() = default;

    void Texture::bind(VkDevice logicalDevice, uint32_t memoryTypeIndex, VkDeviceSize size) {
        m_image.bind(logicalDevice, memoryTypeIndex, size, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    void Texture::createDescriptor(VkDevice logicalDevice, VkDescriptorPool descriptorPool, VkSampler sampler,
                                   VkDescriptorSetLayout descriptorSetLayout) {
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
        imageInfo.sampler = sampler;

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

} // namespace core
