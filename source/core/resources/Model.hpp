#ifndef VULKAN_COURSE_MESHMODEL_HPP
#define VULKAN_COURSE_MESHMODEL_HPP


#include <vector>
#include <string>

#include "vulkan/vulkan.h"
#include "glm/glm.hpp"
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

#include "../mesh/Mesh.hpp"
#include "../renderer/Device.hpp"


namespace core {

    class Model {
    public:
        struct Node {
            uint id;
            std::string name;
            glm::mat4 matrix;
            std::vector<Node> children;
            uint64_t mesh;
            Node* parent;
        };

    public:
        Model();

        explicit Model(std::vector<Node> nodes);

        ~Model();

        Node& getNode(uint id);

        std::vector<Node>& getNodes();

        void cleanup();

        void loadNode(const tinygltf::Node& inputNode, const tinygltf::Model& inputModel, core::Model::Node* parent = nullptr);

        void loadChildren(const tinygltf::Node& inputNode, const tinygltf::Model& inputModel, core::Model::Node* parent);

    private:
        std::vector<Node> m_nodes;
    };

} // namespace core


#endif
