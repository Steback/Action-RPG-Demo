#ifndef PROTOTYPE_ACTION_RPG_TEXTUREMANAGER_HPP
#define PROTOTYPE_ACTION_RPG_TEXTUREMANAGER_HPP


#include <vector>

#include "vulkan/vulkan.h"

#include "Texture.hpp"
#include "../renderer/Device.hpp"
#include "../Utilities.hpp"


namespace core {

    class TextureManager {
    public:
        explicit TextureManager(vk::Device* device, VkQueue graphicsQueue);

        ~TextureManager();

        void cleanup();

        uint createTexture(const std::string& fileName);

        core::Texture& getTexture(size_t index);

        VkDescriptorSet getTextureDescriptorSet(size_t index);

        VkDescriptorSetLayout getDescriptorSetLayout();

        void recreateResources();

        void cleanupResources();

    private:
        void createDescriptorPool();

        void createDescriptorSetLayout();

        void createTextureSampler();

    private:
        vk::Device* m_device{};
        VkQueue m_graphicsQueue{};
        std::vector<core::Texture> textures;
        VkDescriptorPool m_descriptorPool{};
        VkDescriptorSetLayout m_descriptorSetLayout{};
        VkSampler m_textureSampler{};
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_TEXTUREMANAGER_HPP
