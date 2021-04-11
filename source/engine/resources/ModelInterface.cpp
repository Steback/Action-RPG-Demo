#include "ModelInterface.hpp"

#include <utility>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "fmt/format.h"

#include "../Utilities.hpp"
#include "../Application.hpp"


namespace engine {

    ModelInterface::ModelInterface() = default;

    ModelInterface::ModelInterface(std::string name) : m_name(std::move(name)) {

    }

    ModelInterface::ModelInterface(std::vector<Node> nodes, std::string name)
            : m_nodes(std::move(nodes)), m_name(std::move(name)) {

    }

    ModelInterface::~ModelInterface() = default;

    ModelInterface::Node &ModelInterface::getNode(uint id) {
        return m_nodes[id];
    }

    ModelInterface::Node &ModelInterface::getBaseMesh() {
        return m_nodes[m_baseMesh];
    }

    std::vector<ModelInterface::Node>& ModelInterface::getNodes() {
        return m_nodes;
    }

    void ModelInterface::loadNode(const tinygltf::Node &inputNode, const tinygltf::Model &inputModel, int parentID) {
        glm::mat4 matrix(1.0f);
        ModelInterface::Node node{};
        node.id = m_nodes.size();
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
            uint64_t textureID = engine::tools::hashString(image.name);

            if (inputNode.mesh == inputModel.meshes.size() - 1) m_baseMesh = node.id;

            node.mesh = engine::Application::m_resourceManager->loadMesh(node.name, mesh, inputModel, textureID);
        }

        if (parentID != -1) {
            auto& parent = m_nodes[parentID];
            node.parent = parent.id;

            // TODO: Find a better solution for matrix attributes bug
            auto it = std::find(m_conflictsNodes.begin(), m_conflictsNodes.end(), node.name);
            if (it == m_conflictsNodes.end()) node.matrix = parent.matrix * node.matrix;

            parent.children.push_back(node.id);
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

    std::string &ModelInterface::getName() {
        return m_name;
    }

} // namespace core
