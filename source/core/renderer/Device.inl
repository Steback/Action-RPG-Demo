
namespace vk {

    template<typename T>
    void Device::copyData(const Buffer& buffer, const T* pData, const VkDeviceSize& size) {
        void* data;

        vkMapMemory(mDevice, buffer.mDeviceMemory, 0, size, 0, &data);
            memcpy(data, pData, static_cast<size_t>(size));
        vkUnmapMemory(mDevice, buffer.mDeviceMemory);
    }

    template <typename T>
    void Device::createBuffer(Buffer& buffer, const VkBufferUsageFlags& flags,
                              const VkMemoryPropertyFlags& properties, const T* data, const VkDeviceSize& size,
                              VkCommandPool& commandPool, VkQueue queue) {
        vk::Buffer stagingBuffer;

        createBuffer(stagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, size);

        copyData(stagingBuffer, data, size);

        createBuffer(buffer, flags | VK_BUFFER_USAGE_TRANSFER_DST_BIT, properties, size);

        copyBuffer(stagingBuffer.mBuffer, buffer.mBuffer, commandPool, queue, size);

        destroyBuffer(stagingBuffer);
    }

} // end namespace vk