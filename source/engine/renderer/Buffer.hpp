#ifndef PROTOTYPE_ACTION_RPG_BUFFER_HPP
#define PROTOTYPE_ACTION_RPG_BUFFER_HPP


#include <cstring>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "vulkan/vulkan.hpp"


namespace engine {

    class Buffer {
    public:
        Buffer();

        ~Buffer();

        void map(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);

        void unmap();

        void bind(vk::DeviceSize offset = 0) const;

        void setupDescriptor(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);

        template<typename T>
        void copyTo(T *data, vk::DeviceSize size) const {
            std::memcpy(m_mapped, data, size);
        }

        [[nodiscard]] vk::Result flush(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0) const;

        void destroy() const;

        vk::DescriptorBufferInfo& getDescriptorBufferInfo();

    public:
        vk::Device m_device = {};
        vk::Buffer m_buffer = nullptr;
        vk::DeviceMemory m_memory = nullptr;
        vk::DescriptorBufferInfo m_descriptor{};
        vk::DescriptorSet m_descriptorSet{};
        vk::DeviceSize m_size = 0;
        void* m_mapped = nullptr;
    };

} // End namespace vk


#endif //PROTOTYPE_ACTION_RPG_BUFFER_HPP
