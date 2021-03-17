#include "Model.hpp"

#include <utility>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

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
        glm::mat4 nodeMatrix(1.0f);
        Model::Node node{};
        node.name = inputNode.name;

        if (inputNode.translation.size() == 3)
            nodeMatrix = glm::translate(nodeMatrix, glm::make_vec3(reinterpret_cast<const float*>(inputNode.translation.data())));

        if (inputNode.rotation.size() == 4)
            nodeMatrix *= glm::mat4(glm::quat(glm::make_quat(inputNode.rotation.data())));

        if (inputNode.scale.size() == 3)
            nodeMatrix = glm::scale(nodeMatrix, glm::make_vec3(reinterpret_cast<const float*>(inputNode.scale.data())));

        node.matrix = nodeMatrix;

        if (inputNode.mesh > -1) {
            const tinygltf::Mesh& mesh = inputModel.meshes[inputNode.mesh];
            const tinygltf::Material material = inputModel.materials[mesh.primitives[0].material];
            const tinygltf::Texture texture = inputModel.textures[material.pbrMetallicRoughness.baseColorTexture.index];
            const tinygltf::Image image = inputModel.images[texture.source];
            uint64_t textureID = core::tools::hashString(image.name);
            uint64_t meshID = core::tools::hashString(node.name);

            core::Application::resourceManager->loadMesh(meshID, mesh, inputModel, textureID);
            node.mesh = meshID;
        }

        if (parent) {
            node.id = parent->children.size();
            node.parent = parent;
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
