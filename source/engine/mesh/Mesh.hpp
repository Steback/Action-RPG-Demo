#ifndef PROTOTYPE_ACTION_RPG_MESH_HPP
#define PROTOTYPE_ACTION_RPG_MESH_HPP


#include <vector>
#include <memory>

#include "glm/glm.hpp"

#include "Vertex.hpp"
#include "../renderer/Buffer.hpp"
#include "../renderer/Device.hpp"


namespace engine {

    class Mesh {
    public:
        Mesh();

        Mesh(const std::vector<engine::Vertex>& vertices, const std::vector<uint32_t>& indices,
             vk::Queue transferQueue, uint64_t textureID, const std::shared_ptr<engine::Device>& device);

        ~Mesh();

        [[nodiscard]] int getVertexCount() const;

        [[nodiscard]] vk::Buffer getVertexBuffer() const;

        void cleanup();

        [[nodiscard]] int getIndexCount() const;

        [[nodiscard]] vk::Buffer getIndexBuffer() const;

        [[nodiscard]] uint64_t getTextureId() const;

    private:
        void createVertexBuffer(const std::vector<engine::Vertex>& vertices, vk::Queue transferQueue, const std::shared_ptr<engine::Device>& device);

        void createIndexBuffer(const std::vector<uint32_t>& indices, vk::Queue transferQueue, const std::shared_ptr<engine::Device>& device);

    private:
        int m_vertexCount{};
        int m_indexCount{};
        engine::Buffer m_vertexBuffer;
        engine::Buffer m_indexBuffer;
        uint64_t m_textureID{};
    };

} // namespace core


#endif
