#ifndef PROTOTYPE_ACTION_RPG_DEVICE_HPP
#define PROTOTYPE_ACTION_RPG_DEVICE_HPP


#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

#include "SwapChain.hpp"
#include "Buffer.hpp"
#include "Initializers.hpp"
#include "../mesh/Mesh.hpp"
#include "../window/Window.hpp"


namespace vk {

    struct QueueFamilyIndices {
        uint32_t graphics;
        uint32_t present;
        uint32_t compute;
        uint32_t transfer;
    };

    class Device {
    public:
        explicit Device(VkPhysicalDevice physicalDevice);

        ~Device();

        void destroy() const;

        explicit operator VkDevice() const;

        [[nodiscard]] uint32_t getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties) const;

        [[nodiscard]] uint32_t getQueueFamilyIndex(VkQueueFlagBits queueFlags) const;

        bool extensionSupported(const std::string& extension);

        VkResult createLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures,
                                     const std::vector<const char *>& enabledExtensions,
                                     const std::vector<const char *>& validationLayers,
                                     VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);

        [[nodiscard]] VkCommandPool createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) const;

        VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin = false) const;

        [[nodiscard]] VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin = false) const;

        void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free = true) const;

        void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true) const;

        VkResult createBuffer(VkBufferUsageFlags usageFlags,
                              VkMemoryPropertyFlags memoryPropertyFlags,
                              vk::Buffer *buffer,
                              VkDeviceSize size,
                              void *data = nullptr) const;

        void copyBuffer(vk::Buffer *src, vk::Buffer *dst, VkQueue queue, VkBufferCopy *copyRegion = nullptr) const;

    public:
        VkPhysicalDevice m_physicalDevice;
        VkDevice m_logicalDevice{};
        VkPhysicalDeviceProperties m_properties{};
        VkPhysicalDeviceFeatures m_features{};
        VkPhysicalDeviceFeatures m_enabledFeatures{};
        VkPhysicalDeviceMemoryProperties m_memoryProperties{};
        std::vector<VkQueueFamilyProperties> m_queueFamilyProperties;
        std::vector<std::string> m_supportedExtensions;
        VkCommandPool m_commandPool = VK_NULL_HANDLE;
        QueueFamilyIndices m_queueFamilyIndices{};
    };

} // End namespace vk


#endif //PROTOTYPE_ACTION_RPG_DEVICE_HPP
