#include "Model.hpp"

#include <utility>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "../Utilities.hpp"


namespace core {

    Model::Model() = default;

    Model::Model(const core::Mesh& mesh, std::vector<Node> nodes) : m_mesh(mesh), m_nodes(std::move(nodes)) {

    }

    Model::~Model() = default;

    core::Mesh& Model::getMesh() {
        return m_mesh;
    }

    Model::Node &Model::getNode(uint id) {
        return m_nodes[id];
    }

    void Model::cleanup() {
        m_mesh.cleanup();
    }

    void Model::loadNode(const tinygltf::Node& node, const tinygltf::Model& model, uint& meshNodeID, std::vector<Node>& nodes, Node* parent) {
        glm::mat4 modelMatrix(1.0f);
        Node modelNode{};
        modelNode.id = nodes.size();
        modelNode.name = node.name;

        if (node.translation.size() == 3)
            modelMatrix = glm::translate(modelMatrix, glm::make_vec3(reinterpret_cast<const float*>(node.translation.data())));

        if (node.rotation.size() == 4)
            modelMatrix *= glm::mat4(glm::quat(glm::make_quat(node.rotation.data())));

        if (node.scale.size() == 3)
            modelMatrix = glm::scale(modelMatrix, glm::make_vec3(reinterpret_cast<const float*>(node.scale.data())));

        modelNode.mModel = modelMatrix;

        if (node.mesh > -1) meshNodeID = modelNode.id;

        if (parent) {
            parent->children.push_back(modelNode.id);
            modelNode.mModel *= parent->mModel;
        }

        nodes.push_back(modelNode);

        if (!node.children.empty()) {
            for (size_t i : node.children) {
                loadNode(model.nodes[i], model, meshNodeID, nodes, &nodes[modelNode.id]);
            }
        }
    }

    core::Mesh Model::loadMesh(const vk::Device* device, VkQueue queue, const tinygltf::Mesh& mesh, const tinygltf::Model& model,
                               const std::string& texturesID) {
        std::vector<core::Vertex> vertices;
        std::vector<uint32_t> indices;

        for (auto primitive : mesh.primitives) {
            uint32_t indexCount = 0;

            // Vertices
            {
                const float* positionBuffer = nullptr;
                const float* normalsBuffer = nullptr;
                const float* texCoordsBuffer = nullptr;
                size_t vertexCount = 0;

                if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("POSITION")->second];
                    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];

                    positionBuffer = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                    vertexCount = accessor.count;
                }

                if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
                    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];

                    texCoordsBuffer = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }

                for (size_t v = 0; v < vertexCount; ++v) {
                    core::Vertex vertex{};
                    vertex.position = glm::make_vec3(&positionBuffer[v * 3]);
                    vertex.texCoord = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec2(0.0f);
                    vertex.color = glm::vec3(1.0f);

                    vertices.push_back(vertex);
                }
            }

            // Indices
            {
                const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                indexCount += static_cast<uint32_t>(accessor.count);

                switch (accessor.componentType) {
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
                        auto* buf = new uint32_t[accessor.count];
                        std::memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint32_t));

                        for (size_t index = 0; index < accessor.count; ++index) {
                            indices.push_back(buf[index]);
                        }
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                        auto* buf = new uint16_t[accessor.count];
                        memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint16_t));
                        for (size_t index = 0; index < accessor.count; index++) {
                            indices.push_back(buf[index]);
                        }
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                        auto* buf = new uint8_t[accessor.count];
                        memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint8_t));
                        for (size_t index = 0; index < accessor.count; index++) {
                            indices.push_back(buf[index]);
                        }
                        break;
                    }
                }
            }
        }

        return core::Mesh(vertices, indices, queue, texturesID, device);
    }

} // namespace core
