#include "Texture.hpp"



namespace engine {

    Texture::Texture() = default;

    Texture::Texture(vk::Device logicalDevice, vk::Extent2D size, vk::Format format, vk::ImageTiling tiling,
                     vk::ImageUsageFlags usageFlags, uint32_t mipLevels) {
        m_image = engine::Image(logicalDevice, {
                .imageType = vk::ImageType::e2D,
                .format = format,
                .extent = {
                        .width = size.width,
                        .height = size.height,
                        .depth = 1
                },
                .mipLevels = mipLevels,
                .arrayLayers = 1,
                .samples = vk::SampleCountFlagBits::e1,
                .tiling = tiling,
                .usage = usageFlags,
                .sharingMode = vk::SharingMode::eExclusive,
                .initialLayout = vk::ImageLayout::eUndefined
        });

        createTextureSampler(logicalDevice, mipLevels);
    }

    Texture::~Texture() = default;

    void Texture::bind(vk::Device logicalDevice, uint32_t memoryTypeIndex, vk::DeviceSize size) {
        m_image.bind(logicalDevice, memoryTypeIndex, size, vk::ImageAspectFlagBits::eColor);
    }

    // TODO: Check for move descriptor set in resize window
    void Texture::createDescriptor(vk::Device logicalDevice, vk::DescriptorPool descriptorPool, vk::DescriptorSetLayout descriptorSetLayout) {
        m_descriptorSet = logicalDevice.allocateDescriptorSets({
            .descriptorPool = descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &descriptorSetLayout
        }).front();

        vk::DescriptorImageInfo imageInfo{
            .sampler = m_sampler,
            .imageView = m_image.getView(),
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
        };

        vk::WriteDescriptorSet writeDescriptorSet{
            .dstSet = m_descriptorSet,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .pImageInfo = &imageInfo
        };

        logicalDevice.updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
    }

    void Texture::cleanup(vk::Device logicalDevice) {
        vkDestroySampler(logicalDevice, m_sampler, nullptr);
        m_image.cleanup(logicalDevice);
    }

    uint32_t Texture::getWidth() const {
        return m_image.getWidth();
    }

    uint32_t Texture::getHeight() const {
        return m_image.getHeight();
    }

    engine::Image Texture::getTextureImage() const {
        return m_image;
    }

    vk::ImageView Texture::getImageView() const {
        return m_image.getView();
    }

    uint32_t Texture::getMipLevel() const {
        return m_image.getMipLevel();
    }

    vk::DescriptorSet Texture::getDescriptorSet() const {
        return m_descriptorSet;
    }

    void Texture::createTextureSampler(vk::Device logicalDevice, uint32_t mipLevels) {

        m_sampler = logicalDevice.createSampler({
            .magFilter = vk::Filter::eLinear,
            .minFilter = vk::Filter::eLinear,
            .mipmapMode = vk::SamplerMipmapMode::eLinear,
            .addressModeU = vk::SamplerAddressMode::eRepeat,
            .addressModeV = vk::SamplerAddressMode::eRepeat,
            .addressModeW = vk::SamplerAddressMode::eRepeat,
            .mipLodBias = 0.0f,
            .anisotropyEnable = VK_FALSE,
            .maxAnisotropy = 1.0f,
            .compareEnable = VK_FALSE,
            .compareOp = vk::CompareOp::eAlways,
            .minLod = 0.0f,
            .maxLod = static_cast<float>(mipLevels),
            .borderColor = vk::BorderColor::eIntOpaqueBlack,
            .unnormalizedCoordinates = VK_FALSE,
        });
    }

} // namespace core
