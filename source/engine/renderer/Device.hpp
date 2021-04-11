#ifndef PROTOTYPE_ACTION_RPG_DEVICE_HPP
#define PROTOTYPE_ACTION_RPG_DEVICE_HPP


#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "vulkan/vulkan.hpp"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

#include "SwapChain.hpp"
#include "Buffer.hpp"
#include "../window/Window.hpp"


namespace engine {

    class Instance;
    class CommandList;

    struct QueueFamilyIndices {
        uint32_t graphics;
        uint32_t compute;
        uint32_t transfer;
    };

    class Device {
    public:
        explicit Device(const std::shared_ptr<Instance>& instance,
                        vk::QueueFlags requestedQueueTypes = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eTransfer);

        ~Device();

        void destroy() const;

        explicit operator vk::Device() const;

        [[nodiscard]] uint32_t getMemoryType(uint32_t typeBits, vk::MemoryPropertyFlags properties) const;

        [[nodiscard]] uint32_t getQueueFamilyIndex(vk::QueueFlags queueFlags) const;

        [[nodiscard]] vk::CommandPool createCommandPool(const uint32_t* queueFamilyIndex = nullptr, vk::CommandPoolCreateFlags createFlags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer) const;

        [[nodiscard]] vk::CommandBuffer createCommandBuffer(vk::CommandBufferLevel level, vk::CommandPool pool, bool begin = false) const;

        [[nodiscard]] vk::CommandBuffer createCommandBuffer(vk::CommandBufferLevel level, bool begin = false) const;

        void flushCommandBuffer(vk::CommandBuffer commandBuffer, vk::Queue queue, vk::CommandPool pool, bool free = true) const;

        void flushCommandBuffer(vk::CommandBuffer commandBuffer, vk::Queue queue, bool free = true) const;

        engine::Buffer createBuffer(vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memoryPropertyFlags,
                                    vk::DeviceSize size, void *data = nullptr) const;

        void copyBuffer(engine::Buffer *src, engine::Buffer *dst, vk::Queue queue, vk::BufferCopy *copyRegion = nullptr) const;

        void transitionImageLayout(vk::Image image, vk::Format format, vk::Queue queue, vk::ImageLayout oldLayout,
                                   vk::ImageLayout newLayout, uint32_t mipLevels = 1) const;

        void copyBufferToImage(vk::Buffer buffer, vk::Image image, vk::Queue queue, vk::Extent2D size) const;

        [[nodiscard]] vk::SampleCountFlagBits getMaxUsableSampleCount() const;

    public:
        vk::PhysicalDevice m_physicalDevice;
        vk::Device m_logicalDevice{};
        vk::CommandPool m_commandPool = nullptr;
        QueueFamilyIndices m_queueFamilyIndices{};
    };

} // End namespace vk


#endif //PROTOTYPE_ACTION_RPG_DEVICE_HPP
