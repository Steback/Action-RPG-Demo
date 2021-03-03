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
            glm::mat4 mModel;
            std::vector<uint> children;
        };

    public:
        Model();

        explicit Model(const core::Mesh& mesh, std::vector<Node> nodes);

        ~Model();

        core::Mesh& getMesh();

        Node& getNode(uint id);

        void cleanup();

        static void loadNode(const tinygltf::Node& node, const tinygltf::Model& model, uint& meshNodeID, std::vector<Node>& nodes,
                             Node* parent = nullptr);

        static core::Mesh loadMesh(const vk::Device* device, VkQueue queue, const tinygltf::Mesh& mesh, const tinygltf::Model& model,
                                   const std::string& texturesID);

    private:
        core::Mesh m_mesh;
        glm::mat4 m_model{};
        std::vector<Node> m_nodes;
    };

} // namespace core


#endif
