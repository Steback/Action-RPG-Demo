#include <cstring>

#include "Mesh.hpp"


namespace core {

    Mesh::Mesh() = default;

    Mesh::Mesh(const std::vector<core::Vertex>& vertices,const std::vector<uint32_t>& indices,
               VkQueue transferQueue, int newTextureID, const vk::Device* device)
            : m_vertexCount(vertices.size()), m_indexCount(indices.size()), textureID(newTextureID) {
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

    uint Mesh::getTextureId() const {
        return textureID;
    }

    void Mesh::setTextureId(int textureId) {
        textureID = textureId;
    }

    void Mesh::createVertexBuffer(const std::vector<core::Vertex> &vertices, VkQueue transferQueue, const vk::Device* device) {
        VkDeviceSize size = sizeof(core::Vertex) * vertices.size();
        vk::Buffer stagingBuffer;

        device->createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             &stagingBuffer,
                             size,
                             (void*)vertices.data());

        device->createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                             &m_vertexBuffer,
                             size);

        device->copyBuffer(&stagingBuffer, &m_vertexBuffer, transferQueue);

        stagingBuffer.destroy();
    }

    void Mesh::createIndexBuffer(const std::vector<uint32_t> &indices, VkQueue transferQueue, const vk::Device* device) {
        VkDeviceSize size = sizeof(uint32_t) * indices.size();
        vk::Buffer stagingBuffer;

        device->createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             &stagingBuffer,
                             size,
                             (void*)indices.data());

        device->createBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                             &m_indexBuffer,
                             size);

        device->copyBuffer(&stagingBuffer, &m_indexBuffer, transferQueue);

        stagingBuffer.destroy();
    }

} // namespace core
