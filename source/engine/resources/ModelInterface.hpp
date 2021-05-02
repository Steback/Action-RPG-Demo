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

    class ModelInterface {
    public:
        struct Node {
            uint id;
            std::string name;
            glm::mat4 matrix;
            glm::vec3 position;
            glm::quat rotation;
            glm::vec3 scale;
            std::vector<int> children;
            uint64_t mesh;
            int parent;

            glm::mat4 getLocalMatrix() {
                return glm::translate(glm::mat4(1.0f), position) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale) * matrix;
            }
        };

    public:
        ModelInterface();

        ModelInterface(std::string  name);

        explicit ModelInterface(std::vector<Node> nodes, std::string  name);

        ~ModelInterface();

        Node& getNode(uint id);

        Node& getBaseMesh();

        std::vector<Node>& getNodes();

        void loadNode(const tinygltf::Node& inputNode, const tinygltf::Model& inputModel, int parentID = -1);

        std::string& getName();

    private:
        std::vector<Node> m_nodes;
        int m_baseMesh{};
        std::string m_name{};
    };

} // namespace core


#endif
