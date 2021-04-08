#include "Image.hpp"


namespace core {

    Image::Image() = default;

    Image::Image(vk::Device logicalDevice, const vk::ImageCreateInfo& createInfo) {
        m_format = createInfo.format;
        m_mipLevels = createInfo.mipLevels;
        m_extent = createInfo.extent;

        m_image = logicalDevice.createImage(createInfo);
    }

    Image::~Image() = default;

    void Image::bind(vk::Device logicalDevice, uint32_t memoryTypeIndex, vk::DeviceSize size, vk::ImageAspectFlagBits aspectFlags) {
        m_memory = logicalDevice.allocateMemory({
            .allocationSize = size,
            .memoryTypeIndex = memoryTypeIndex
        });

        logicalDevice.bindImageMemory(m_image, m_memory, 0);

        // Create an image view after the image has been bound to GPU memory
        vk::ImageViewCreateInfo viewInfo{
            .image = m_image,
            .viewType = vk::ImageViewType::e2D,
            .format = m_format,
            .subresourceRange = {
                    .aspectMask = aspectFlags,
                    .baseMipLevel = 0,
                    .levelCount = m_mipLevels,
                    .baseArrayLayer = 0,
                    .layerCount = 1
            }
        };

        m_view = logicalDevice.createImageView(viewInfo);
    }

    void Image::cleanup(vk::Device logicalDevice) {
        vkDestroyImageView(logicalDevice, m_view, nullptr);
        vkDestroyImage(logicalDevice, m_image, nullptr);
        vkFreeMemory(logicalDevice, m_memory, nullptr);
    }

    vk::Format Image::getFormat() const {
        return m_format;
    }

    uint32_t Image::getWidth() const {
        return m_extent.width;
    }

    uint32_t Image::getHeight() const {
        return m_extent.height;
    }

    vk::Image Image::getImage() const {
        return m_image;
    }

    vk::ImageView Image::getView() const {
        return m_view;
    }

    uint32_t Image::getMipLevel() const {
        return m_mipLevels;
    }

    void Image::createImageView(vk::Device logicalDevice, vk::ImageAspectFlagBits aspectFlags) {

    }

} // namespace vk
