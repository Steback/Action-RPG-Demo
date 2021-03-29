#ifndef PROTOTYPE_ACTION_RPG_MESH_HPP
#define PROTOTYPE_ACTION_RPG_MESH_HPP


#include <vector>
#include <memory>

#include "glm/glm.hpp"

#include "Vertex.hpp"
#include "../renderer/Buffer.hpp"
#include "../renderer/Device.hpp"


namespace core {

    class Mesh {
    public:
        Mesh();

        Mesh(const std::vector<core::Vertex>& vertices,const std::vector<uint32_t>& indices,
             VkQueue transferQueue, uint64_t textureID, const std::shared_ptr<vkc::Device>& device);

        ~Mesh();

        [[nodiscard]] int getVertexCount() const;

        [[nodiscard]] VkBuffer getVertexBuffer() const;

        void cleanup();

        [[nodiscard]] int getIndexCount() const;

        [[nodiscard]] VkBuffer getIndexBuffer() const;

        [[nodiscard]] uint64_t getTextureId() const;

    private:
        void createVertexBuffer(const std::vector<core::Vertex>& vertices, VkQueue transferQueue, const std::shared_ptr<vkc::Device>& device);

        void createIndexBuffer(const std::vector<uint32_t>& indices, VkQueue transferQueue, const std::shared_ptr<vkc::Device>& device);

    private:
        int m_vertexCount{};
        int m_indexCount{};
        vkc::Buffer m_vertexBuffer;
        vkc::Buffer m_indexBuffer;
        uint64_t m_textureID{};
    };

} // namespace core


#endif
