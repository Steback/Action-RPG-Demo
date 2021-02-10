#ifndef PROTOTYPE_ACTION_RPG_RESOURCEMANAGER_HPP
#define PROTOTYPE_ACTION_RPG_RESOURCEMANAGER_HPP


#include <vector>

#include "vulkan/vulkan.h"

#include "Texture.hpp"
#include "../renderer/Device.hpp"
#include "../components/Model.hpp"
#include "../Utilities.hpp"


namespace core {

    class ResourceManager {
    public:
        explicit ResourceManager(vk::Device* device, VkQueue graphicsQueue);

        ~ResourceManager();

        void cleanup();

        uint createTexture(const std::string& fileName);

        core::Texture& getTexture(size_t index);

        VkDescriptorSetLayout& getTextureDescriptorSetLayout();

        void recreateResources();

        void cleanupResources();

        void generateMipmaps(const core::Texture& texture, VkFormat format, VkExtent2D size, uint32_t mipLevels);

        core::Model createModel(const std::string& fileName);

    private:
        void createDescriptorPool();

        void createDescriptorSetLayout();

    private:
        vk::Device* m_device{};
        VkQueue m_graphicsQueue{};
        std::vector<core::Texture> textures;
        VkDescriptorPool m_descriptorPool{};
        VkDescriptorSetLayout m_descriptorSetLayout{};
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_RESOURCEMANAGER_HPP
