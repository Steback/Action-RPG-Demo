#include "Buffer.hpp"

#include "../Utilities.hpp"


namespace core {

    Buffer::Buffer() = default;

    Buffer::~Buffer() = default;

    void Buffer::map(vk::DeviceSize size, vk::DeviceSize offset) {
        m_mapped = m_device.mapMemory(m_memory, offset, size);
    }

    void Buffer::unmap() {
        if (m_mapped) {
            m_device.unmapMemory(m_memory);
            m_mapped = nullptr;
        }
    }

    void Buffer::bind(vk::DeviceSize offset) const {
        m_device.bindBufferMemory(m_buffer, m_memory, offset);
    }

    void Buffer::setupDescriptor(vk::DeviceSize size, vk::DeviceSize offset) {
        m_descriptor.offset = offset;
        m_descriptor.buffer = m_buffer;
        m_descriptor.range = size;
    }

    vk::Result Buffer::flush(vk::DeviceSize size, vk::DeviceSize offset) const {
        vk::MappedMemoryRange mappedRange{
            .memory = m_memory,
            .offset = offset,
            .size = size
        };

        return m_device.flushMappedMemoryRanges(1, &mappedRange);
    }

    void Buffer::destroy() const {
        if (m_buffer) m_device.destroyBuffer(m_buffer);

        if (m_memory) m_device.freeMemory(m_memory);
    }

} // End namespace vk