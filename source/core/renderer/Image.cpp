#include "Image.hpp"

#include "Initializers.hpp"
#include "Tools.hpp"


namespace vk {

    Image::Image() = default;

    Image::Image(VkDevice logicalDevice, const VkImageCreateInfo& createInfo) {
        createImage(logicalDevice, createInfo);
    }

    Image::~Image() = default;

    void Image::bind(VkDevice logicalDevice, uint32_t memoryTypeIndex, VkDeviceSize size, VkImageAspectFlagBits aspectFlags) {
        VkMemoryAllocateInfo allocInfo = vk::initializers::memoryAllocateInfo();
        allocInfo.allocationSize = size;
        allocInfo.memoryTypeIndex = memoryTypeIndex;

        vk::tools::validation(vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &m_memory),
                              "Failed to allocate image memory");

        vkBindImageMemory(logicalDevice, m_image, m_memory, 0);

        // Create an image view after the image has been bound to GPU memory
        createImageView(logicalDevice, aspectFlags);
    }

    void Image::cleanup(VkDevice logicalDevice) {
        vkDestroyImageView(logicalDevice, m_view, nullptr);
        vkDestroyImage(logicalDevice, m_image, nullptr);
        vkFreeMemory(logicalDevice, m_memory, nullptr);
    }

    VkFormat Image::getFormat() const {
        return m_format;
    }

    uint32_t Image::getWidth() const {
        return m_extent.width;
    }

    uint32_t Image::getHeight() const {
        return m_extent.height;
    }

    VkImage Image::getImage() const {
        return m_image;
    }

    VkImageView Image::getView() const {
        return m_view;
    }

    uint32_t Image::getMipLevel() const {
        return m_mipLevels;
    }

    void Image::createImage(VkDevice logicalDevice, const VkImageCreateInfo &createInfo) {
        m_format = createInfo.format;
        m_mipLevels = createInfo.mipLevels;
        m_extent = createInfo.extent;

        vk::tools::validation(vkCreateImage(logicalDevice, &createInfo, nullptr, &m_image),
                              "Failed to create image");
    }

    void Image::createImageView(VkDevice logicalDevice, VkImageAspectFlagBits aspectFlags) {
        VkImageViewCreateInfo viewInfo = vk::initializers::imageViewCreateInfo();
        viewInfo.image = m_image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = m_mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        vk::tools::validation(vkCreateImageView(logicalDevice, &viewInfo, nullptr, &m_view),
                              "Failed to create image view");
    }

} // namespace vk
