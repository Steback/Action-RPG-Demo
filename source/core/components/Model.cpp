#include "Model.hpp"

#include <utility>


namespace core {

    Model::Model() = default;

    Model::Model(std::vector<core::Mesh> meshList) : m_meshList(std::move(meshList)) {

    }

    Model::~Model() = default;

    size_t Model::getMeshCount() const {
        return m_meshList.size();
    }

    core::Mesh *Model::getMesh(size_t index) {
        if (index >= m_meshList.size()) {
            throw std::runtime_error("Attempted to access invalid Mesh Index");
        }

        return &m_meshList[index];
    }

    void Model::clean() {
        for (auto& mesh : m_meshList) {
            mesh.cleanup();
        }
    }

    std::vector<std::string> Model::loadMaterials(const aiScene *scene) {
        // Create 1:1 sized list of textures
        std::vector<std::string> textureList(scene->mNumMaterials);

        // Go through each material and copy its texture file name (if it exists)
        for (size_t i = 0; i < scene->mNumMaterials; ++i) {
            // Get the material
            aiMaterial * material = scene->mMaterials[i];

            // Initialise the texture to empty string (will be replaced if texture exists)
            textureList[i] = "";

            // Check for a Diffuse Texture (standard detail texture)
            if (material->GetTextureCount(aiTextureType_DIFFUSE)) {
                // Get the path of the texture file
                aiString path;

                if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS) {
                    // Cut off any directory information already present
                    int idx = std::string(path.data).rfind('\\');
                    std::string fileName = std::string(path.data).substr(idx + 1);

                    textureList[i] = fileName;
                }
            }
        }

        return textureList;
    }

    std::vector<Mesh>Model::LoadNode(const vk::Device* device, VkQueue queue, aiNode *node, const aiScene *scene,
                                     const std::vector<uint>& matToTex) {
        std::vector<core::Mesh> meshList;

        // Go through each mesh at this node and create it, then add it to our meshList
        for (size_t i = 0; i < node->mNumMeshes; ++i) {
            meshList.push_back(
                    LoadMesh(device, queue, scene->mMeshes[node->mMeshes[i]], scene, matToTex)
            );
        }

        // Go through each node attached to this node and load it, then append their meshes to this node's mesh list
        for (size_t i = 0; i < node->mNumChildren; ++i) {
            std::vector<Mesh> newList = LoadNode(device, queue, node->mChildren[i], scene, matToTex);
            meshList.insert(meshList.end(), newList.begin(), newList.end());
        }

        return meshList;
    }

    core::Mesh Model::LoadMesh(const vk::Device* device, VkQueue queue, aiMesh *mesh, const aiScene *scene,
                               const std::vector<uint>& matToTex) {
        std::vector<core::Vertex> vertices;
        std::vector<uint32_t> indices;

        // Resize vertex list to hold all vertices for mesh
        vertices.resize(mesh->mNumVertices);

        // Go through each vertex and copy it across to our vertices
        for (size_t i = 0; i < mesh->mNumVertices; ++i) {
            // Set m_position
            vertices[i].position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };

            // Set tex coords (if they exist)
            if (mesh->mTextureCoords[0]) {
                vertices[i].texCoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
            } else {
                vertices[i].texCoord = { 0.0f, 0.0f };
            }

            // Set colour (just use white for now)
            vertices[i].color = { 1.0f, 1.0f, 1.0f };
        }

        // Iterate over indices through faces and copy across
        for (size_t i = 0; i < mesh->mNumFaces; ++i) {
            // Get a face
            aiFace face = mesh->mFaces[i];

            // Go through face's indices and add to list
            for (size_t j = 0; j < face.mNumIndices; ++j) {
                indices.push_back(face.mIndices[j]);
            }
        }

        // Create new mesh with details and return it
        Mesh newMesh = core::Mesh(vertices, indices, queue, matToTex[mesh->mMaterialIndex], device);

        return newMesh;
    }

} // namespace core
