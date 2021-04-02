#include <cstring>

#include "Mesh.hpp"


namespace core {

    Mesh::Mesh() = default;

    Mesh::Mesh(const std::vector<core::Vertex>& vertices,const std::vector<uint32_t>& indices,
               VkQueue transferQueue, uint64_t textureID, const std::shared_ptr<vkc::Device>& device)
            : m_vertexCount(vertices.size()), m_indexCount(indices.size()), m_textureID(textureID) {
        createVertexBuffer(vertices, transferQueue, device);
        createIndexBuffer(indices, transferQueue, device);
    }

    Mesh::~Mesh() = default;

    int Mesh::getVertexCount() const {
        return m_vertexCount;
    }

    VkBuffer Mesh::getVertexBuffer() const {
        return m_vertexBuffer.m_buffer;
    }

    int Mesh::getIndexCount() const {
        return m_indexCount;
    }

    VkBuffer Mesh::getIndexBuffer() const {
        return m_indexBuffer.m_buffer;
    }

    void Mesh::cleanup() {
        m_indexBuffer.destroy();
        m_vertexBuffer.destroy();
    }

    uint64_t Mesh::getTextureId() const {
        return m_textureID;
    }

    void Mesh::createVertexBuffer(const std::vector<core::Vertex> &vertices, VkQueue transferQueue, const std::shared_ptr<vkc::Device>& device) {
        VkDeviceSize size = sizeof(core::Vertex) * vertices.size();
        vkc::Buffer stagingBuffer;

        device->createBuffer(vk::BufferUsageFlagBits::eTransferSrc,
                             vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                             &stagingBuffer,
                             size,
                             (void*)vertices.data());

        device->createBuffer(vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                             vk::MemoryPropertyFlagBits::eDeviceLocal,
                             &m_vertexBuffer,
                             size);

        device->copyBuffer(&stagingBuffer, &m_vertexBuffer, transferQueue);

        stagingBuffer.destroy();
    }

    void Mesh::createIndexBuffer(const std::vector<uint32_t> &indices, VkQueue transferQueue, const std::shared_ptr<vkc::Device>& device) {
        VkDeviceSize size = sizeof(uint32_t) * indices.size();
        vkc::Buffer stagingBuffer;

        device->createBuffer(vk::BufferUsageFlagBits::eTransferSrc,
                             vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                             &stagingBuffer,
                             size,
                             (void*)indices.data());

        device->createBuffer(vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                             vk::MemoryPropertyFlagBits::eDeviceLocal,
                             &m_indexBuffer,
                             size);

        device->copyBuffer(&stagingBuffer, &m_indexBuffer, transferQueue);

        stagingBuffer.destroy();
    }

} // namespace core
