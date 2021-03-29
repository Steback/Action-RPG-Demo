#include "Buffer.hpp"

#include "../Utilities.hpp"


namespace vkc {

    Buffer::Buffer() = default;

    Buffer::~Buffer() = default;

    VkResult Buffer::map(VkDeviceSize size, VkDeviceSize offset) {
        return vkMapMemory(m_device, m_memory, offset, size, 0, &m_mapped);
    }

    void Buffer::unmap() {
        if (m_mapped) {
            vkUnmapMemory(m_device, m_memory);
            m_mapped = nullptr;
        }
    }

    VkResult Buffer::bind(VkDeviceSize offset) const {
        return vkBindBufferMemory(m_device, m_buffer, m_memory, offset);
    }

    void Buffer::setupDescriptor(VkDeviceSize size, VkDeviceSize offset) {
        m_descriptor.offset = offset;
        m_descriptor.buffer = m_buffer;
        m_descriptor.range = size;
    }

    VkResult Buffer::flush(VkDeviceSize size, VkDeviceSize offset) const {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = m_memory;
        mappedRange.offset = offset;
        mappedRange.size = size;

        return vkFlushMappedMemoryRanges(m_device, 1, &mappedRange);
    }

    VkResult Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset) const {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = m_memory;
        mappedRange.offset = offset;
        mappedRange.size = size;

        return vkInvalidateMappedMemoryRanges(m_device, 1, &mappedRange);
    }

    void Buffer::destroy() const {
        if (m_buffer) vkDestroyBuffer(m_device, m_buffer, nullptr);

        if (m_memory) vkFreeMemory(m_device, m_memory, nullptr);
    }

} // End namespace vk