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
        Model();

        explicit Model(std::vector<core::Mesh>  meshList);

        ~Model();

        [[nodiscard]] size_t getMeshCount() const;

        core::Mesh* getMesh(size_t index);

        glm::mat4& getMatrixModel();

        void setModel(const glm::mat4& model);

        void clean();

        static std::vector<core::Mesh> loadNode(const vk::Device* device, VkQueue queue, const tinygltf::Node& node,
                                                const tinygltf::Model& model, const std::vector<uint>& texturesID);

        static core::Mesh loadMesh(const vk::Device* device, VkQueue queue, const tinygltf::Mesh& mesh,
                                   const tinygltf::Model& model, const std::vector<uint>& texturesID);

    private:
        std::vector<core::Mesh> m_meshList;
        glm::mat4 m_model{};
    };

} // namespace core


#endif
