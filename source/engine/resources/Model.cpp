#include "Model.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "fmt/format.h"

#include "../Utilities.hpp"
#include "../Application.hpp"


const std::vector<std::string> conflictsNodes = {
        "Bow",
        "fould_A",
        "fould_B",
        "helmet_A",
        "helmet_B",
        "helmet_C",
        "hood_A",
        "skeleton_mesh"
};

namespace engine {

    Model::Model() = default;

    Model::Model(std::string name) : m_name(std::move(name)) {

    }

    Model::Model(std::vector<Node> nodes, std::string name)
            : m_nodes(std::move(nodes)), m_name(std::move(name)) {

    }

    Model::~Model() = default;

    Model::Node &Model::getNode(uint id) {
        return m_nodes[id];
    }

    std::vector<Model::Node>& Model::getNodes() {
        return m_nodes;
    }

    void Model::loadNode(const tinygltf::Node &inputNode, const tinygltf::Model &inputModel, int parentID) {
        Node node{};
        glm::mat4 matrix = glm::mat4(1.0f);
        node.id = m_nodes.size();
        node.name = inputNode.name;

        if (inputNode.translation.size() == 3) {
            node.position = glm::make_vec3(inputNode.translation.data());
            matrix = glm::translate(matrix, node.position);
        }

        if (inputNode.rotation.size() == 4) {
            glm::quat rotation = glm::make_quat(inputNode.rotation.data());
            node.rotation = glm::mat4(rotation);
            matrix *= glm::mat4(node.rotation);
        }

        if (inputNode.scale.size() == 3) {
            node.scale = glm::make_vec3(inputNode.scale.data());
            matrix = glm::scale(matrix, node.scale);
        }

        if (inputNode.matrix.size() == 16) {
            node.matrix = matrix * glm::mat4(glm::make_mat4x4(inputNode.matrix.data()));
        } else {
            node.matrix = matrix;
        }

        if (inputNode.mesh > -1) {
            const tinygltf::Mesh& mesh = inputModel.meshes[inputNode.mesh];
            const tinygltf::Material material = inputModel.materials[mesh.primitives[0].material];
            const tinygltf::Texture texture = inputModel.textures[material.pbrMetallicRoughness.baseColorTexture.index];
            const tinygltf::Image image = inputModel.images[texture.source];
            uint64_t textureID = engine::tools::hashString(image.name);

            node.mesh = engine::Application::m_resourceManager->loadMesh(node.name, mesh, inputModel, textureID);
        }

        if (parentID != -1) {
            auto& parent = m_nodes[parentID];
            parent.children.push_back(static_cast<int>(parent.id));
            node.parent = static_cast<int>(parent.id);

            // TODO: All the skeletons models have a error when applying transforms with some nodes.
            // So my "best" solution to this problem. I create a vector with all the name of the conflict nodes and
            // not apply the transform to them. If anyone can tell what's the error or how to solve it, I really appreciate that.
            if (std::find(conflictsNodes.begin(), conflictsNodes.end(), node.name) == conflictsNodes.end())
                node.matrix = parent.matrix * node.matrix;
        } else {
            node.parent = -1;
        }

        m_nodes.push_back(node);

        if (!inputNode.children.empty()) {
            for (size_t i : inputNode.children) {
                loadNode(inputModel.nodes[i], inputModel, node.id);
            }
        }
    }

    std::string &Model::getName() {
        return m_name;
    }

} // namespace core
