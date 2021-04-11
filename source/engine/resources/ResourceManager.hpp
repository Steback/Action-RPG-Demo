#ifndef PROTOTYPE_ACTION_RPG_RESOURCEMANAGER_HPP
#define PROTOTYPE_ACTION_RPG_RESOURCEMANAGER_HPP


#include <vector>
#include <memory>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "vulkan/vulkan.hpp"

#include "Texture.hpp"
#include "../renderer/Device.hpp"
#include "ModelInterface.hpp"
#include "../Utilities.hpp"


namespace engine {

    class Shader;

    class ResourceManager {
    public:
        explicit ResourceManager(std::shared_ptr<engine::Device> device, vk::Queue graphicsQueue);

        ~ResourceManager();

        void cleanup();

        void createTexture(const std::string& uri, const std::string& name);

        engine::Texture& getTexture(uint64_t id);

        vk::DescriptorSetLayout& getTextureDescriptorSetLayout();

        void recreateResources();

        void cleanupResources();

        void generateMipmaps(const engine::Texture& texture, vk::Format format, vk::Extent2D size, uint32_t mipLevels);

        uint64_t createModel(const std::string& uri, const std::string& name);

        std::shared_ptr<engine::ModelInterface> getModel(uint64_t id);

        engine::Mesh& getMesh(uint64_t id);

        uint64_t loadMesh(const std::string& name, const tinygltf::Mesh& mesh, const tinygltf::Model& model, uint64_t texturesID);

        std::shared_ptr<engine::Shader> createShader(const std::string &vert, const std::string &frag, const std::vector<vk::PushConstantRange>& pushConstants, bool vertexInfo = true);

    private:
        void createDescriptorPool();

        void createDescriptorSetLayout();

    private:
        std::shared_ptr<engine::Device> m_device{};
        vk::Queue m_graphicsQueue{};
        std::unordered_map<uint64_t, engine::Texture> m_textures;
        std::unordered_map<uint64_t, std::shared_ptr<engine::ModelInterface>> m_models;
        std::unordered_map<uint64_t, engine::Mesh> m_meshes;
        std::vector<std::shared_ptr<engine::Shader>> m_shaders;
        vk::DescriptorPool m_descriptorPool{};
        vk::DescriptorSetLayout m_descriptorSetLayout{};
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_RESOURCEMANAGER_HPP
