#ifndef VULKAN_COURSE_MESHMODEL_HPP
#define VULKAN_COURSE_MESHMODEL_HPP


#include <vector>
#include <string>

#include "vulkan/vulkan.h"
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

#include "../Constants.hpp"
#include "../mesh/Mesh.hpp"
#include "../renderer/Device.hpp"


namespace engine {

    class Model {
    public:
        struct Node {
            uint32_t id{};
            std::string name{};
            glm::mat4 matrix{};
            glm::vec3 position{};
            glm::quat rotation{};
            glm::vec3 scale{1.0f};
            std::vector<uint32_t> children;
            uint64_t mesh{};
            int32_t parent{-1};
            int32_t skin{-1};

            glm::mat4 getLocalMatrix() const;

            glm::mat4 getMatrix(const std::shared_ptr<Model>& model) const;
        };

        struct Skin {
            std::string name{};
            int32_t rootNodeID{};
            std::vector<glm::mat4> inverseBindMatrices;
            std::vector<uint32_t> joints;
        };

    public:
        Model();

        Model(std::string name, int32_t numNodes = -1);

        explicit Model(std::vector<Node> nodes, std::string  name, int32_t numNodes = -1);

        ~Model();

        void cleanup();

        Node& getNode(uint32_t id);

        std::vector<Node>& getNodes();

        void loadNode(const tinygltf::Node& inputNode, const tinygltf::Model& inputModel, uint32_t nodeID, int32_t parentID = -1);

        std::string& getName();

        uint32_t getRootNode() const;

        void loadSkins(const tinygltf::Model& inputModel, const std::shared_ptr<Device>& device, const vk::Queue& transfer);

        Skin& getSkin(size_t i);

        uint32_t getSkinsCount() const;

    private:
        std::vector<Node> m_nodes;
        std::string m_name{};
        uint32_t m_rootNode{};
        std::vector<Skin> m_skins;
    };

} // namespace core


#endif
