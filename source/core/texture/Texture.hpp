#ifndef PROTOTYPE_ACTION_RPG_TEXTURE_HPP
#define PROTOTYPE_ACTION_RPG_TEXTURE_HPP


#include <string>

#include "../renderer/Image.hpp"
#include "../renderer/Device.hpp"

namespace core {

    class Texture {
    public:
        Texture();

        Texture(VkDevice logicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                VkImageUsageFlags usageFlags);

        ~Texture();

        void bind(VkDevice logicalDevice, uint32_t memoryTypeIndex, VkDeviceSize size);

        void createDescriptor(VkDevice logicalDevice, VkDescriptorPool descriptorPool, VkSampler sampler,
                              VkDescriptorSetLayout descriptorSetLayout);

        void cleanup(VkDevice logicalDevice);

        [[nodiscard]] uint32_t getWidth() const;

        [[nodiscard]] uint32_t getHeight() const;

        [[nodiscard]] vk::Image getTextureImage() const;

        [[nodiscard]] VkImageView getImageView() const;

        [[nodiscard]] uint32_t getMipLevel() const;

        VkDescriptorSet getDescriptorSet() const;

    private:
        vk::Image m_image;
        VkDescriptorSet m_descriptorSet{};
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_TEXTURE_HPP
