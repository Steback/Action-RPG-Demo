#ifndef PROTOTYPE_ACTION_RPG_BUFFER_HPP
#define PROTOTYPE_ACTION_RPG_BUFFER_HPP


#include <cstring>

#include "vulkan/vulkan.h"


namespace vkc {

    class Buffer {
    public:
        Buffer();

        ~Buffer();

        VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        void unmap();

        [[nodiscard]] VkResult bind(VkDeviceSize offset = 0) const;

        void setupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        template<typename T>
        void copyTo(T *data, VkDeviceSize size) const {
            std::memcpy(m_mapped, data, size);
        }

        VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;

        [[nodiscard]] VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;

        void destroy() const;

    public:
        VkDevice m_device = {};
        VkBuffer m_buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_memory = VK_NULL_HANDLE;
        VkDescriptorBufferInfo m_descriptor{};
        VkDeviceSize m_size = 0;
        VkDeviceSize m_alignment = 0;
        VkBufferUsageFlags m_usageFlags{};
        VkMemoryPropertyFlags m_memoryPropertyFlags{};
        void* m_mapped = nullptr;
    };

} // End namespace vk


#endif //PROTOTYPE_ACTION_RPG_BUFFER_HPP
