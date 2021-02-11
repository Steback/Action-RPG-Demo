#include "Model.hpp"

#include <utility>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "../Utilities.hpp"


namespace core {

    Model::Model() = default;

    Model::Model(std::vector<core::Mesh>  meshList) : m_meshList(std::move(meshList)) {

    }

    Model::~Model() = default;

    size_t Model::getMeshCount() const {
        return m_meshList.size();
    }

    core::Mesh *Model::getMesh(size_t index) {
        if (index >= m_meshList.size()) {
            core::throw_ex("Attempted to access invalid Mesh Index");
        }

        return &m_meshList[index];
    }

    glm::mat4 &Model::getMatrixModel() {
        return m_model;
    }

    void Model::setModel(const glm::mat4 &model) {
        m_model = model;
    }

    void Model::clean() {
        for (auto& mesh : m_meshList) {
            mesh.cleanup();
        }
    }

    std::vector<core::Mesh> Model::loadNode(const vk::Device* device, VkQueue queue, const tinygltf::Node& node,
                                            const tinygltf::Model& model, const std::vector<uint>& texturesID) {
        std::vector<core::Mesh> meshes;

        if (!node.children.empty()) {
            for (size_t i : node.children) {
                auto meshList = loadNode(device, queue, model.nodes[i], model, texturesID);
                meshes.insert(meshes.end(), meshList.begin(), meshList.end());
            }
        }

        if (node.mesh > -1) {
            meshes.push_back(loadMesh(device, queue, model.meshes[node.mesh], model, texturesID));
        }

        return meshes;
    }

    core::Mesh Model::loadMesh(const vk::Device* device, VkQueue queue, const tinygltf::Mesh& mesh, const tinygltf::Model& model,
                               const std::vector<uint>& texturesID) {
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

        return core::Mesh(vertices, indices, queue, texturesID[0], device);
    }

} // namespace core
