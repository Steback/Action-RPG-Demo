#include "Model.hpp"

#include <utility>
#include <glm/gtx/matrix_decompose.inl>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "fmt/format.h"

#include "../Utilities.hpp"
#include "../Application.hpp"


namespace core {

    Model::Model() = default;

    Model::Model(std::vector<Node> nodes)
            : m_nodes(std::move(nodes)) {

    }

    Model::~Model() = default;

    Model::Node &Model::getNode(uint id) {
        return m_nodes[id];
    }

    std::vector<Model::Node>& Model::getNodes() {
        return m_nodes;
    }

    void Model::cleanup() {

    }

    void Model::loadNode(const tinygltf::Node &inputNode, const tinygltf::Model &inputModel, core::Model::Node* parent) {
        glm::mat4 matrix(1.0f);
        Model::Node node{};
        node.name = inputNode.name;

        if (inputNode.translation.size() == 3) {
            glm::vec3 translation = glm::make_vec3(inputNode.translation.data());

            matrix = glm::translate(matrix, translation);
        }

        if (inputNode.rotation.size() == 4) {
            glm::quat rotation = glm::make_quat(inputNode.rotation.data());

            matrix *= glm::mat4(rotation);
        }

        if (inputNode.scale.size() == 3) {
            glm::vec3 scale = glm::make_vec3(inputNode.scale.data());

            matrix = glm::scale(matrix, scale);
        }

        node.matrix = matrix;

        if (inputNode.mesh > -1) {
            const tinygltf::Mesh& mesh = inputModel.meshes[inputNode.mesh];
            const tinygltf::Material material = inputModel.materials[mesh.primitives[0].material];
            const tinygltf::Texture texture = inputModel.textures[material.pbrMetallicRoughness.baseColorTexture.index];
            const tinygltf::Image image = inputModel.images[texture.source];
            uint64_t textureID = core::tools::hashString(image.name);
            uint64_t meshID = core::tools::hashString(node.name);

            node.mesh = core::Application::resourceManager->loadMesh(node.name, mesh, inputModel, textureID);
        }

        if (parent) {
            node.id = parent->children.size();
            node.parent = parent;

            // TODO: Find a better solution for matrix attributes bug
            auto it = std::find(m_conflictsNodes.begin(), m_conflictsNodes.end(), node.name);

            if (it == m_conflictsNodes.end()) node.matrix = parent->matrix * node.matrix;

            parent->children.push_back(node);

            loadChildren(inputNode, inputModel, &parent->children[node.id]);
        } else {
            node.id = m_nodes.size();
            node.parent = nullptr;
            m_nodes.push_back(node);

            loadChildren(inputNode, inputModel, &m_nodes[node.id]);
        }
    }

    void Model::loadChildren(const tinygltf::Node& inputNode, const tinygltf::Model& inputModel, core::Model::Node* parent) {
        if (!inputNode.children.empty()) {
            for (size_t i : inputNode.children) {
                loadNode(inputModel.nodes[i], inputModel, parent);
            }
        }
    }

} // namespace core
