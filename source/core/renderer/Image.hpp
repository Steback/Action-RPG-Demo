#ifndef PROTOTYPE_ACTION_RPG_IMAGE_HPP
#define PROTOTYPE_ACTION_RPG_IMAGE_HPP


#include "vulkan/vulkan.h"


namespace vk {

    class Image {
    public:
        Image();

        Image(VkDevice logicalDevice, const VkImageCreateInfo& createInfo);

        ~Image();

        void bind(VkDevice logicalDevice, uint32_t memoryTypeIndex, VkDeviceSize size, VkImageAspectFlagBits aspectFlags);

        void cleanup(VkDevice logicalDevice);

        [[nodiscard]] VkFormat getFormat() const;

        [[nodiscard]] uint32_t getWidth() const;

        [[nodiscard]] uint32_t getHeight() const;

        [[nodiscard]] VkImage getImage() const;

        [[nodiscard]] VkImageView getView() const;

        [[nodiscard]] uint32_t getMipLevel() const;

    private:
        void createImage(VkDevice logicalDevice, const VkImageCreateInfo &imageCreateInfo);

        void createImageView(VkDevice logicalDevice, VkImageAspectFlagBits aspectFlags);

    private:
        VkImage m_image = {};
        VkImageView m_view = {};
        VkFormat m_format = {};
        VkDeviceMemory m_memory = {};
        VkExtent3D m_extent = {};
        uint32_t m_mipLevels = 1;
    };

} // namespace vk


#endif //PROTOTYPE_ACTION_RPG_IMAGE_HPP
