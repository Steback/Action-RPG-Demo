#ifndef VULKAN_COURSE_MESHMODEL_HPP
#define VULKAN_COURSE_MESHMODEL_HPP


#include <vector>
#include <string>

#include "assimp/scene.h"
#include "vulkan/vulkan.h"
#include "glm/glm.hpp"

#include "../mesh/Mesh.hpp"
#include "../renderer/Device.hpp"


namespace core {

    class Model {
    public:
        Model();

        explicit Model(std::vector<core::Mesh> meshList);

        ~Model();

        [[nodiscard]] size_t getMeshCount() const;

        core::Mesh* getMesh(size_t index);

        void clean();

        static std::vector<std::string> loadMaterials(const aiScene* scene);

        static std::vector<core::Mesh> LoadNode(const vk::Device* device, VkQueue queue, aiNode *node, const aiScene *scene,
                                                const std::vector<uint>& matToTex);

        static core::Mesh LoadMesh(const vk::Device* device, VkQueue queue, aiMesh* mesh, const aiScene* scene,
                                    const std::vector<uint>& matToTex);

    private:
        std::vector<core::Mesh> m_meshList;
    };

} // namespace core


#endif
