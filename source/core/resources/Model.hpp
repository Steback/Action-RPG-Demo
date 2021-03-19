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


namespace core {

    const std::vector<std::string> m_conflictsNodes = {
            "Bow",
            "fould_A",
            "fould_B",
            "helmet_A",
            "helmet_B",
            "helmet_C",
            "hood_A",
            "skeleton_mesh"
    };

    class Model {
    public:
        struct Node {
            uint id;
            std::string name;
            glm::mat4 matrix;
            std::vector<int> children;
            uint64_t mesh;
            int parent;
        };

    public:
        Model();

        explicit Model(std::vector<Node> nodes);

        ~Model();

        Node& getNode(uint id);

        Node& getBaseMesh();

        std::vector<Node>& getNodes();

        void cleanup();

        void loadNode(const tinygltf::Node& inputNode, const tinygltf::Model& inputModel, int parentID = -1);

    private:
        std::vector<Node> m_nodes;
        int m_baseMesh;
    };

} // namespace core


#endif
