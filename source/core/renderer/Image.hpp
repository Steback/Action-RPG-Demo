#ifndef PROTOTYPE_ACTION_RPG_IMAGE_HPP
#define PROTOTYPE_ACTION_RPG_IMAGE_HPP


#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "vulkan/vulkan.hpp"


namespace core {

    class Image {
    public:
        Image();

        Image(vk::Device logicalDevice, const vk::ImageCreateInfo& createInfo);

        ~Image();

        void bind(vk::Device logicalDevice, uint32_t memoryTypeIndex, vk::DeviceSize size, vk::ImageAspectFlagBits aspectFlags);

        void cleanup(vk::Device logicalDevice);

        [[nodiscard]] vk::Format getFormat() const;

        [[nodiscard]] uint32_t getWidth() const;

        [[nodiscard]] uint32_t getHeight() const;

        [[nodiscard]] vk::Image getImage() const;

        [[nodiscard]] vk::ImageView getView() const;

        [[nodiscard]] uint32_t getMipLevel() const;

    private:

        void createImageView(vk::Device logicalDevice, vk::ImageAspectFlagBits aspectFlags);

    private:
        vk::Image m_image = {};
        vk::ImageView m_view = {};
        vk::Format m_format = {};
        vk::DeviceMemory m_memory = {};
        vk::Extent3D m_extent = {};
        uint32_t m_mipLevels = 1;
    };

} // namespace vk


#endif //PROTOTYPE_ACTION_RPG_IMAGE_HPP
