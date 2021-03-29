#ifndef PROTOTYPE_ACTION_RPG_RESOURCEMANAGER_HPP
#define PROTOTYPE_ACTION_RPG_RESOURCEMANAGER_HPP


#include <vector>
#include <memory>

#include "vulkan/vulkan.h"

#include "Texture.hpp"
#include "../renderer/Device.hpp"
#include "Model.hpp"
#include "../Utilities.hpp"


namespace core {

    class ResourceManager {
    public:
        explicit ResourceManager(std::shared_ptr<vkc::Device> device, VkQueue graphicsQueue);

        ~ResourceManager();

        void cleanup();

        void createTexture(const std::string& uri, const std::string& name);

        core::Texture& getTexture(uint64_t id);

        VkDescriptorSetLayout& getTextureDescriptorSetLayout();

        void recreateResources();

        void cleanupResources();

        void generateMipmaps(const core::Texture& texture, VkFormat format, VkExtent2D size, uint32_t mipLevels);

        uint64_t createModel(const std::string& uri, const std::string& name);

        std::shared_ptr<core::Model> getModel(uint64_t id);

        core::Mesh& getMesh(uint64_t id);

        uint64_t loadMesh(const std::string& name, const tinygltf::Mesh& mesh, const tinygltf::Model& model, uint64_t texturesID);

    private:
        void createDescriptorPool();

        void createDescriptorSetLayout();

    private:
        std::shared_ptr<vkc::Device> m_device{};
        VkQueue m_graphicsQueue{};
        std::unordered_map<uint64_t, core::Texture> m_textures;
        std::unordered_map<uint64_t, std::shared_ptr<core::Model>> m_models;
        std::unordered_map<uint64_t, core::Mesh> m_meshes;
        VkDescriptorPool m_descriptorPool{};
        VkDescriptorSetLayout m_descriptorSetLayout{};
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_RESOURCEMANAGER_HPP
