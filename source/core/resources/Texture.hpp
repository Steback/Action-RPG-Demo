#ifndef PROTOTYPE_ACTION_RPG_TEXTURE_HPP
#define PROTOTYPE_ACTION_RPG_TEXTURE_HPP


#include <string>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "vulkan/vulkan.hpp"

#include "../renderer/Image.hpp"


namespace core {

    class Texture {
    public:
        Texture();

        Texture(vk::Device logicalDevice, vk::Extent2D size, vk::Format format, vk::ImageTiling tiling,
                vk::ImageUsageFlags usageFlags, uint32_t mipLevels = 1);

        ~Texture();

        void bind(vk::Device logicalDevice, uint32_t memoryTypeIndex, vk::DeviceSize size);

        void createDescriptor(vk::Device logicalDevice, vk::DescriptorPool descriptorPool, vk::DescriptorSetLayout descriptorSetLayout);

        void cleanup(vk::Device logicalDevice);

        [[nodiscard]] uint32_t getWidth() const;

        [[nodiscard]] uint32_t getHeight() const;

        [[nodiscard]] core::Image getTextureImage() const;

        [[nodiscard]] vk::ImageView getImageView() const;

        [[nodiscard]] uint32_t getMipLevel() const;

        vk::DescriptorSet getDescriptorSet() const;

    private:
        void createTextureSampler(vk::Device logicalDevice, uint32_t mipLevels);

    private:
        core::Image m_image;
        vk::DescriptorSet m_descriptorSet{};
        vk::Sampler m_sampler{};
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_TEXTURE_HPP
