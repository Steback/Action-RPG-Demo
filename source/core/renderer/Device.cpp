#include "Device.hpp"

#include "Instance.hpp"
#include "../Utilities.hpp"


namespace vkc {

    Device::Device(const std::shared_ptr<Instance>& instance, vk::QueueFlags requestedQueueTypes) {
        std::vector<const char*> reqExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };

        m_physicalDevice = instance->selectPhysicalDevice(reqExtensions);
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos{};

        const float defaultQueuePriority(0.0f);

        // Graphics queue
        if (requestedQueueTypes & vk::QueueFlagBits::eGraphics) {
            m_queueFamilyIndices.graphics = getQueueFamilyIndex(vk::QueueFlagBits::eGraphics);

            queueCreateInfos.push_back({
                .queueFamilyIndex = m_queueFamilyIndices.graphics,
                .queueCount = 1,
                .pQueuePriorities = &defaultQueuePriority
            });
        } else {
            m_queueFamilyIndices.graphics = VK_NULL_HANDLE;
        }

        // Dedicated compute queue
        if (requestedQueueTypes & vk::QueueFlagBits::eCompute) {
            m_queueFamilyIndices.compute = getQueueFamilyIndex(vk::QueueFlagBits::eCompute);

            if (m_queueFamilyIndices.compute != m_queueFamilyIndices.graphics) {
                // If compute family index differs, we need an additional queue create info for the compute queue
                queueCreateInfos.push_back({
                    .queueFamilyIndex = m_queueFamilyIndices.compute,
                    .queueCount = 1,
                    .pQueuePriorities = &defaultQueuePriority
                });
            }
        } else {
            // Else we use the same queue
            m_queueFamilyIndices.compute = m_queueFamilyIndices.graphics;
        }

        // Dedicated transfer queue
        if (requestedQueueTypes & vk::QueueFlagBits::eTransfer) {
            m_queueFamilyIndices.transfer = getQueueFamilyIndex(vk::QueueFlagBits::eTransfer);

            if ((m_queueFamilyIndices.transfer != m_queueFamilyIndices.graphics)
                    && (m_queueFamilyIndices.transfer != m_queueFamilyIndices.compute)) {
                // If compute family index differs, we need an additional queue create info for the compute queue
                queueCreateInfos.push_back({
                    .queueFamilyIndex = m_queueFamilyIndices.transfer,
                    .queueCount = 1,
                    .pQueuePriorities = &defaultQueuePriority
                });
            }
        } else {
            // Else we use the same queue
            m_queueFamilyIndices.transfer = m_queueFamilyIndices.graphics;
        }

        m_logicalDevice = m_physicalDevice.createDevice({
            .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledExtensionCount = static_cast<uint32_t>(reqExtensions.size()),
            .ppEnabledExtensionNames = reqExtensions.data(),
            .pEnabledFeatures = {},
        });

        // Create a default command pool for graphics command buffers
        m_commandPool = createCommandPool(m_queueFamilyIndices.graphics);
    }

    Device::~Device() = default;

    void Device::destroy() const {
        if (m_commandPool) m_logicalDevice.destroyCommandPool(m_commandPool);

        if (m_logicalDevice) m_logicalDevice.destroy();
    }

    Device::operator vk::Device() const {
        return m_logicalDevice;
    }

    uint32_t Device::getMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const {
        vk::PhysicalDeviceMemoryProperties memProperties = m_physicalDevice.getMemoryProperties();

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
                return i;
        }

        throw std::runtime_error("Failed to find suitable memory type");

        return -1;
    }

    uint32_t Device::getQueueFamilyIndex(vk::QueueFlags queueFlags) const {
        std::vector<vk::QueueFamilyProperties> properties = m_physicalDevice.getQueueFamilyProperties();

        // Dedicated queue for compute
        // Try to find a queue family index that supports compute but not graphics
        if (queueFlags & vk::QueueFlagBits::eCompute) {
            for (uint32_t i = 0; i < static_cast<uint32_t>(properties.size()); i++) {
                if ((properties[i].queueFlags & queueFlags)
                        && (!(properties[i].queueFlags & vk::QueueFlagBits::eGraphics)))
                    return i;
            }
        }

        // Dedicated queue for transfer
        // Try to find a queue family index that supports transfer but not graphics and compute
        if (queueFlags & vk::QueueFlagBits::eTransfer) {
            for (uint32_t i = 0; i < static_cast<uint32_t>(properties.size()); i++) {
                if ((properties[i].queueFlags & queueFlags)
                        && (!(properties[i].queueFlags & vk::QueueFlagBits::eCompute))
                        && (!(properties[i].queueFlags & vk::QueueFlagBits::eCompute)))
                    return i;
            }
        }

        // For other queue types or if no separate compute queue is present, return the first one to support the requested flags
        for (uint32_t i = 0; i < static_cast<uint32_t>(properties.size()); i++) {
            if (properties[i].queueFlags & queueFlags)
                return i;
        }

        throw std::runtime_error("Could not find a matching queue family index");

        return -1;
    }

    vk::CommandPool Device::createCommandPool(uint32_t queueFamilyIndex, vk::CommandPoolCreateFlags createFlags) const {
        return m_logicalDevice.createCommandPool({
            .flags = createFlags,
            .queueFamilyIndex = queueFamilyIndex
        });
    }

    vk::CommandBuffer Device::createCommandBuffer(vk::CommandBufferLevel level, vk::CommandPool pool, bool begin) const {
        vk::CommandBuffer cmdBuffer;

        cmdBuffer = m_logicalDevice.allocateCommandBuffers({
            .commandPool = pool,
            .level = level,
            .commandBufferCount = 1,
        }).front();

        // If requested, also start recording for the new command buffer
        if (begin) {
            cmdBuffer.begin({
                .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
            });
        }

        return cmdBuffer;
    }

    vk::CommandBuffer Device::createCommandBuffer(vk::CommandBufferLevel level, bool begin) const {
        return createCommandBuffer(level, m_commandPool, begin);
    }

    void Device::flushCommandBuffer(vk::CommandBuffer commandBuffer, vk::Queue queue, vk::CommandPool pool, bool free) const {
        if (!commandBuffer) return;

        commandBuffer.end();

        vk::SubmitInfo submitInfo{
                .commandBufferCount = 1,
                .pCommandBuffers = &commandBuffer
        };

        // Create fence to ensure that the command buffer has finished executing
        vk::Fence fence = m_logicalDevice.createFence({});

        // Submit to the queue
        VK_CHECK_RESULT_HPP(queue.submit(1, &submitInfo, fence))

        // Wait for the fence to signal that command buffer has finished executing
        VK_CHECK_RESULT_HPP(m_logicalDevice.waitForFences(1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max()))

        m_logicalDevice.destroy(fence);

        if (free)
            m_logicalDevice.freeCommandBuffers(pool, commandBuffer);
    }

    void Device::flushCommandBuffer(vk::CommandBuffer commandBuffer, vk::Queue queue, bool free) const {
        return flushCommandBuffer(commandBuffer, queue, m_commandPool, free);
    }

     vkc::Buffer Device::createBuffer(vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memoryPropertyFlags,
                                      vk::DeviceSize size, void *data) const {
        vkc::Buffer buffer;
        buffer.m_device = m_logicalDevice;

        // Create the buffer handle
        buffer.m_buffer = m_logicalDevice.createBuffer({
            .size = size,
            .usage = usageFlags
        });

        // Create the memory backing up the buffer handle
        vk::MemoryRequirements memReqs = m_logicalDevice.getBufferMemoryRequirements(buffer.m_buffer);
        vk::MemoryAllocateInfo memAlloc{
            .allocationSize = memReqs.size,
            .memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits , memoryPropertyFlags)
        };

        vk::MemoryAllocateFlagsInfoKHR allocFlagsInfo{};
        if (usageFlags & vk::BufferUsageFlagBits::eShaderDeviceAddress) {
            allocFlagsInfo.flags = vk::MemoryAllocateFlagBits::eDeviceAddress;
            memAlloc.pNext = &allocFlagsInfo;
        }

        buffer.m_memory = m_logicalDevice.allocateMemory(memAlloc);

        buffer.m_size = size;

        // If a pointer to the buffer data has been passed, map the buffer and copy over the data
        if (data != nullptr) {
            buffer.map(size);

            buffer.copyTo(data, size);

            if (!(memoryPropertyFlags & vk::MemoryPropertyFlagBits::eHostCoherent))
                VK_CHECK_RESULT_HPP(buffer.flush(size))

            buffer.unmap();
        }

        // Initialize a default descriptor that covers the whole buffer size
        buffer.setupDescriptor(size);

        // Attach the memory to the buffer object
        buffer.bind();

         return buffer;
    }

    void Device::copyBuffer(vkc::Buffer *src, vkc::Buffer *dst, vk::Queue queue, vk::BufferCopy *copyRegion) const {
        vk::CommandBuffer copyCmd = createCommandBuffer(vk::CommandBufferLevel::ePrimary, true);
        vk::BufferCopy bufferCopy{};

        if (copyRegion == nullptr) {
            bufferCopy.size = src->m_size;
        } else {
            bufferCopy = *copyRegion;
        }

        copyCmd.copyBuffer(src->m_buffer, dst->m_buffer, 1, &bufferCopy);

        flushCommandBuffer(copyCmd, queue);
    }

    void Device::transitionImageLayout(vk::Image image, vk::Format format, vk::Queue queue, vk::ImageLayout oldLayout,
                                       vk::ImageLayout newLayout, uint32_t mipLevels) const {
        vk::CommandBuffer commandBuffer = createCommandBuffer(vk::CommandBufferLevel::ePrimary, true);

        vk::ImageMemoryBarrier barrier{
            .srcAccessMask = {},
            .dstAccessMask = {},
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .baseMipLevel = 0,
                    .levelCount = mipLevels,
                    .baseArrayLayer = 0,
                    .layerCount = 1
            }
        };

        vk::PipelineStageFlags sourceStage;
        vk::PipelineStageFlags destinationStage;

        if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
            barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;

            // Check for stencil support
            if (format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint) {
                barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
            }
        } else {
            barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        }

        if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
            barrier.srcAccessMask = {};
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

            sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
            destinationStage = vk::PipelineStageFlagBits::eTransfer;
        } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            sourceStage = vk::PipelineStageFlagBits::eTransfer;
            destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
        } else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
            barrier.srcAccessMask = {};
            barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

            sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
            destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
        } else {
            throw std::runtime_error("Unsupported layout transition!");
        }

        commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, 0, nullptr, 0, nullptr, 1, &barrier);

        flushCommandBuffer(commandBuffer, queue);
    }

    void Device::copyBufferToImage(vk::Buffer buffer, vk::Image image, vk::Queue queue, vk::Extent2D size) const {
        vk::CommandBuffer commandBuffer = createCommandBuffer(vk::CommandBufferLevel::ePrimary, true);

        vk::BufferImageCopy region{
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = 1
            },
            .imageOffset = {0, 0, 0},
            .imageExtent = {
                    .width = size.width,
                    .height = size.height,
                    .depth = 1
            }
        };

        commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);

        flushCommandBuffer(commandBuffer, queue);
    }

    vk::SampleCountFlagBits Device::getMaxUsableSampleCount() const {
        vk::PhysicalDeviceProperties properties = m_physicalDevice.getProperties();

        vk::SampleCountFlags counts = properties.limits.framebufferColorSampleCounts &
                properties.limits.framebufferDepthSampleCounts;

        if (counts & vk::SampleCountFlagBits::e64) { return vk::SampleCountFlagBits::e64; }
        if (counts & vk::SampleCountFlagBits::e32) { return vk::SampleCountFlagBits::e32; }
        if (counts & vk::SampleCountFlagBits::e16) { return vk::SampleCountFlagBits::e16; }
        if (counts & vk::SampleCountFlagBits::e8) { return vk::SampleCountFlagBits::e8; }
        if (counts & vk::SampleCountFlagBits::e4) { return vk::SampleCountFlagBits::e4; }
        if (counts & vk::SampleCountFlagBits::e2) { return vk::SampleCountFlagBits::e2; }

        return vk::SampleCountFlagBits::e1;
    }

} // End namespace vk
