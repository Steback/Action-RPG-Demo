#ifndef PROTOTYPE_ACTION_RPG_TEXTURE_HPP
#define PROTOTYPE_ACTION_RPG_TEXTURE_HPP


#include <string>

#include "../renderer/Image.hpp"
#include "../renderer/Device.hpp"

namespace core {

    class Texture {
    public:
        Texture();

        Texture(VkDevice logicalDevice, VkExtent2D size, VkFormat format, VkImageTiling tiling,
                VkImageUsageFlags usageFlags, uint32_t mipLevels = 1);

        ~Texture();

        void bind(VkDevice logicalDevice, uint32_t memoryTypeIndex, VkDeviceSize size);

        void createDescriptor(VkDevice logicalDevice, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout);

        void cleanup(VkDevice logicalDevice);

        [[nodiscard]] uint32_t getWidth() const;

        [[nodiscard]] uint32_t getHeight() const;

        [[nodiscard]] vk::Image getTextureImage() const;

        [[nodiscard]] VkImageView getImageView() const;

        [[nodiscard]] uint32_t getMipLevel() const;

        VkDescriptorSet getDescriptorSet() const;

    private:

        void createTextureSampler(VkDevice logicalDevice, uint32_t mipLevels);

    private:
        vk::Image m_image;
        VkDescriptorSet m_descriptorSet{};
        VkSampler m_sampler{};
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_TEXTURE_HPP
