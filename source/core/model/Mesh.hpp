#ifndef VULKAN_COURSE_MESH_HPP
#define VULKAN_COURSE_MESH_HPP


#include <vector>

#include "glm/glm.hpp"

#include "Vertex.hpp"
#include "../renderer/Buffer.hpp"
#include "../renderer/Device.hpp"


namespace core {

    class Mesh {
    public:
        Mesh();

        Mesh(const std::vector<core::Vertex>& vertices,const std::vector<uint32_t>& indices,
             VkQueue transferQueue, int newTextureID, const vk::Device* device);

        ~Mesh();

        [[nodiscard]] int getVertexCount() const;

        VkBuffer getVertexBuffer() const;

        void cleanup();

        [[nodiscard]] int getIndexCount() const;

        VkBuffer getIndexBuffer() const;

        [[nodiscard]] const glm::mat4 &getUboModel() const;

        void setUboModel(const glm::mat4 &uboModel);

        [[nodiscard]] uint getTextureId() const;

        void setTextureId(int textureId);

    private:
        void createVertexBuffer(const std::vector<core::Vertex>& vertices, VkQueue transferQueue, const vk::Device* device);

        void createIndexBuffer(const std::vector<uint32_t>& indices, VkQueue transferQueue, const vk::Device* device);

    private:
        glm::mat4 m_model{};
        int m_vertexCount{};
        int m_indexCount{};
        vk::Buffer m_vertexBuffer;
        vk::Buffer m_indexBuffer;
        uint textureID{};
    };

} // namespace core


#endif
