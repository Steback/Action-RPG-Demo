#include "Mesh.hpp"

#include "fmt/format.h"


namespace engine {

    Mesh::Mesh() = default;

    Mesh::Mesh(const std::vector<engine::Vertex>& vertices, const std::vector<uint32_t>& indices,
               vk::Queue transferQueue, uint64_t textureID, const std::shared_ptr<engine::Device>& device)
            : m_vertexCount(vertices.size()), m_indexCount(indices.size()), m_textureID(textureID) {
        createVertexBuffer(vertices, transferQueue, device);
        createIndexBuffer(indices, transferQueue, device);

        vk::DeviceSize size = sizeof(m_uniformBlock);
        m_uniformBuffer = device->createBuffer(vk::BufferUsageFlagBits::eUniformBuffer,
                                               vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                                               size);
        m_uniformBuffer.map(size);
        m_uniformBuffer.setupDescriptor(size);
    }

    Mesh::~Mesh() = default;

    int Mesh::getVertexCount() const {
        return m_vertexCount;
    }

    vk::Buffer Mesh::getVertexBuffer() const {
        return m_vertexBuffer.m_buffer;
    }

    int Mesh::getIndexCount() const {
        return m_indexCount;
    }

    vk::Buffer Mesh::getIndexBuffer() const {
        return m_indexBuffer.m_buffer;
    }

    void Mesh::cleanup() {
        m_uniformBuffer.destroy();
        m_indexBuffer.destroy();
        m_vertexBuffer.destroy();
    }

    uint64_t Mesh::getTextureId() const {
        return m_textureID;
    }

    void Mesh::createVertexBuffer(const std::vector<engine::Vertex> &vertices, vk::Queue transferQueue, const std::shared_ptr<engine::Device>& device) {
        vk::DeviceSize size = sizeof(engine::Vertex) * vertices.size();
        engine::Buffer stagingBuffer;

        stagingBuffer = device->createBuffer(vk::BufferUsageFlagBits::eTransferSrc,
                             vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                             size, (void*)vertices.data());

        m_vertexBuffer = device->createBuffer(vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                             vk::MemoryPropertyFlagBits::eDeviceLocal, size);

        device->copyBuffer(&stagingBuffer, &m_vertexBuffer, transferQueue);

        stagingBuffer.destroy();
    }

    void Mesh::createIndexBuffer(const std::vector<uint32_t> &indices, vk::Queue transferQueue, const std::shared_ptr<engine::Device>& device) {
        vk::DeviceSize size = sizeof(uint32_t) * indices.size();
        engine::Buffer stagingBuffer;

        stagingBuffer = device->createBuffer(vk::BufferUsageFlagBits::eTransferSrc,
                             vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                             size, (void*)indices.data());

        m_indexBuffer = device->createBuffer(vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                             vk::MemoryPropertyFlagBits::eDeviceLocal, size);

        device->copyBuffer(&stagingBuffer, &m_indexBuffer, transferQueue);

        stagingBuffer.destroy();
    }

} // namespace core
