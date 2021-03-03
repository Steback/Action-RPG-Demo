#ifndef PROTOTYPE_ACTION_RPG_RESOURCEMANAGER_HPP
#define PROTOTYPE_ACTION_RPG_RESOURCEMANAGER_HPP


#include <vector>

#include "vulkan/vulkan.h"

#include "Texture.hpp"
#include "../renderer/Device.hpp"
#include "Model.hpp"
#include "../Utilities.hpp"


namespace core {

    class ResourceManager {
    public:
        explicit ResourceManager(vk::Device* device, VkQueue graphicsQueue);

        ~ResourceManager();

        void cleanup();

        void createTexture(const std::string& uri, const std::string& name);

        core::Texture& getTexture(const std::string& name);

        VkDescriptorSetLayout& getTextureDescriptorSetLayout();

        void recreateResources();

        void cleanupResources();

        void generateMipmaps(const core::Texture& texture, VkFormat format, VkExtent2D size, uint32_t mipLevels);

        void createModel(const std::string& uri, const std::string& name, uint& meshNodeID);

        core::Model& getModel(const std::string& id);

    private:
        void createDescriptorPool();

        void createDescriptorSetLayout();

    private:
        vk::Device* m_device{};
        VkQueue m_graphicsQueue{};
        std::unordered_map<std::string, core::Texture> m_textures;
        std::unordered_map<std::string, core::Model> m_models;
        VkDescriptorPool m_descriptorPool{};
        VkDescriptorSetLayout m_descriptorSetLayout{};
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_RESOURCEMANAGER_HPP
