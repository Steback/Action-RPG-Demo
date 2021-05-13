#ifndef PROTOTYPE_ACTION_RPG_RESOURCEMANAGER_HPP
#define PROTOTYPE_ACTION_RPG_RESOURCEMANAGER_HPP


#include <vector>
#include <memory>
#include <mutex>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "vulkan/vulkan.hpp"

#include "Texture.hpp"
#include "Model.hpp"
#include "Animation.hpp"
#include "../Utilities.hpp"
#include "../renderer/Device.hpp"


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

        std::shared_ptr<engine::Model> getModel(uint64_t id);

        engine::Mesh& getMesh(uint64_t id);

        uint64_t loadMesh(const std::string& name, const tinygltf::Mesh& mesh, const tinygltf::Model& model, uint64_t texturesID);

        std::shared_ptr<engine::Shader> createShader(const std::string &vert, const std::string &frag, const std::vector<vk::PushConstantRange>& pushConstants, bool vertexInfo = true);

        Animation& getAnimation(uint64_t name);

        void createSkinsDescriptorSets();

        void createSkinsDescriptors(const std::vector<vk::DescriptorPoolSize>& sizes, uint32_t maxSize);

        uint32_t getSkinsCount();

        vk::DescriptorSetLayout getSkinsDescriptorSetLayout();

    private:
        void createDescriptorPool();

        void createDescriptorSetLayout();

        uint64_t loadAnimation(const std::string& uri, const std::string& name);

    private:
        std::shared_ptr<engine::Device> m_device{};
        vk::Queue m_graphicsQueue{};
        std::unordered_map<uint64_t, engine::Texture> m_textures;
        std::unordered_map<uint64_t, std::shared_ptr<engine::Model>> m_models;
        std::unordered_map<uint64_t, engine::Mesh> m_meshes;
        std::unordered_map<uint64_t, Animation> m_animations;
        std::vector<std::shared_ptr<engine::Shader>> m_shaders;
        vk::DescriptorPool m_imagesDescriptorPool{};
        vk::DescriptorSetLayout m_imagesDescriptorSetLayout{};
        vk::DescriptorPool m_skinSDescriptorPool{};
        vk::DescriptorSetLayout m_skinsDescriptorSetLayout{};
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_RESOURCEMANAGER_HPP
