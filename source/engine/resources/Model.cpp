#include "Model.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "fmt/format.h"

#include "../Utilities.hpp"
#include "../Application.hpp"


namespace engine {

    glm::mat4 Model::Node::getLocalMatrix() const {
        return glm::translate(glm::mat4(1.0f), position) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale) * matrix;
    }

    glm::mat4 Model::Node::getMatrix(const std::shared_ptr<Model>& model) const {
        glm::mat4 nodeMatrix = getLocalMatrix();
        int32_t parenID = parent;

        while (parenID > -1) {
            Model::Node& currentParent = model->getNode(parenID);
            nodeMatrix = currentParent.getLocalMatrix() * nodeMatrix;
            parenID = currentParent.parent;
        }

        return nodeMatrix;
    }

    Model::Model() = default;

    Model::Model(std::string name, int32_t numNodes) : m_name(std::move(name)) {
        if (numNodes != -1) m_nodes.resize(numNodes);
    }

    Model::Model(std::vector<Node> nodes, std::string name, int32_t numNodes)
            : m_nodes(std::move(nodes)), m_name(std::move(name)) {
        if (numNodes != -1) m_nodes.resize(numNodes);
    }

    Model::~Model() = default;

    void Model::cleanup() {

    }

    Model::Node &Model::getNode(uint32_t id) {
        return m_nodes[id];
    }

    std::vector<Model::Node>& Model::getNodes() {
        return m_nodes;
    }

    void Model::loadNode(const tinygltf::Node &inputNode, const tinygltf::Model &inputModel, uint32_t nodeID, int32_t parentID) {
        Node node{};
        node.id = nodeID;
        node.name = inputNode.name;

        if (inputNode.translation.size() == 3) {
            node.position = glm::make_vec3(inputNode.translation.data());
        }

        if (inputNode.rotation.size() == 4) {
            glm::quat rotation = glm::make_quat(inputNode.rotation.data());
            node.rotation = glm::mat4(rotation);
        }

        if (inputNode.scale.size() == 3) {
            node.scale = glm::make_vec3(inputNode.scale.data());
        }

        if (inputNode.matrix.size() == 16) {
            node.matrix = glm::mat4(glm::make_mat4x4(inputNode.matrix.data()));
        } else {
            node.matrix = glm::mat4(1.0f);
        }

        if (inputNode.mesh > -1) {
            const tinygltf::Mesh& mesh = inputModel.meshes[inputNode.mesh];
            const tinygltf::Material material = inputModel.materials[mesh.primitives[0].material];
            const tinygltf::Texture texture = inputModel.textures[material.pbrMetallicRoughness.baseColorTexture.index];
            const tinygltf::Image image = inputModel.images[texture.source];
            uint64_t textureID = engine::tools::hashString(image.name);
            node.mesh = engine::Application::m_resourceManager->loadMesh(node.name, mesh, inputModel, textureID);
        }

        if (inputNode.skin > -1) node.skin = inputNode.skin;

        if (parentID != -1) {
            auto& parent = m_nodes[parentID];
            parent.children.push_back(node.id);
            node.parent = static_cast<int32_t>(parent.id);
        } else {
            m_rootNode = node.id;
        }

        m_nodes[node.id] = node;

        if (!inputNode.children.empty()) {
            for (size_t i : inputNode.children) {
                loadNode(inputModel.nodes[i], inputModel, static_cast<uint32_t>(i), static_cast<int32_t>(node.id));
            }
        }
    }

    std::string &Model::getName() {
        return m_name;
    }

    uint32_t Model::getRootNode() const {
        return m_rootNode;
    }

    void Model::loadSkins(const tinygltf::Model& inputModel, const std::shared_ptr<Device>& device, const vk::Queue& transfer) {
        for (auto& node : m_nodes) {
            if (node.skin > -1) {
                tinygltf::Skin gltfSkin = inputModel.skins[node.skin];

                m_skins.push_back({});
                Skin& skin = m_skins.back();
                skin.name = gltfSkin.name;
                skin.rootNodeID = gltfSkin.skeleton;

                for (int joint : gltfSkin.joints) skin.joints.push_back(joint);

                if (gltfSkin.inverseBindMatrices > -1) {
                    const tinygltf::Accessor &accessor = inputModel.accessors[gltfSkin.inverseBindMatrices];
                    const tinygltf::BufferView &bufferView = inputModel.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer &buffer = inputModel.buffers[bufferView.buffer];
                    skin.inverseBindMatrices.resize(accessor.count);

                    std::memcpy(skin.inverseBindMatrices.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(glm::mat4));
                }
            }
        }
    }

    Model::Skin &Model::getSkin(size_t i) {
        return m_skins[i];
    }

    uint32_t Model::getSkinsCount() const {
        return static_cast<uint32_t>(m_skins.size());
    }
} // namespace core
