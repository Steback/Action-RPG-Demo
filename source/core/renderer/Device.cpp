#include "Device.hpp"

#include "Debug.hpp"
#include "Tools.hpp"


namespace vk {

    Device::Device(VkPhysicalDevice physicalDevice) : m_physicalDevice(physicalDevice) {
        vkGetPhysicalDeviceProperties(m_physicalDevice, &m_properties);
        vkGetPhysicalDeviceFeatures(m_physicalDevice, &m_features);
        vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_memoryProperties);

        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

        m_queueFamilyProperties.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                                 m_queueFamilyProperties.data());

        uint32_t extCount = 0;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, nullptr);

        if (extCount > 0) {
            std::vector<VkExtensionProperties> extensions(extCount);

            if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, &extensions.front())
                    == VK_SUCCESS) {
                for (auto ext : extensions) {
                    m_supportedExtensions.emplace_back(ext.extensionName);
                }
            }
        }
    }

    Device::~Device() = default;

    void Device::destroy() const {
        if (m_commandPool) {
            vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);
        }

        if (m_logicalDevice) {
            vkDestroyDevice(m_logicalDevice, nullptr);
        }
    }

    Device::operator VkDevice() const {
        return m_logicalDevice;
    }

    uint32_t Device::getMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        core::throw_ex("Failed to find suitable memory type");

        return -1;
    }

    uint32_t Device::getQueueFamilyIndex(VkQueueFlagBits queueFlags) const {
        // Dedicated queue for compute
        // Try to find a queue family index that supports compute but not graphics
        if (queueFlags & VK_QUEUE_COMPUTE_BIT) {
            for (uint32_t i = 0; i < static_cast<uint32_t>(m_queueFamilyProperties.size()); i++) {
                if ((m_queueFamilyProperties[i].queueFlags & queueFlags)
                        && ((m_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)) {
                    return i;
                }
            }
        }

        // Dedicated queue for transfer
        // Try to find a queue family index that supports transfer but not graphics and compute
        if (queueFlags & VK_QUEUE_TRANSFER_BIT) {
            for (uint32_t i = 0; i < static_cast<uint32_t>(m_queueFamilyProperties.size()); i++) {
                if ((m_queueFamilyProperties[i].queueFlags & queueFlags)
                        && ((m_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)
                        && ((m_queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)) {
                    return i;
                }
            }
        }

        // For other queue types or if no separate compute queue is present, return the first one to support the requested flags
        for (uint32_t i = 0; i < static_cast<uint32_t>(m_queueFamilyProperties.size()); i++) {
            if (m_queueFamilyProperties[i].queueFlags & queueFlags) {
                return i;
            }
        }

        core::throw_ex("Could not find a matching queue family index");

        return -1;
    }

    bool Device::extensionSupported(const std::string& extension) {
        return (std::find(m_supportedExtensions.begin(), m_supportedExtensions.end(), extension)
                != m_supportedExtensions.end());
    }

    VkResult
    Device::createLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures,
                                const std::vector<const char *>& enabledExtensions,
                                const std::vector<const char *>& validationLayers,
                                VkQueueFlags requestedQueueTypes) {
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

        // Get queue family indices for the requested queue family types
        // Note that the indices may overlap depending on the implementation
        const float defaultQueuePriority(0.0f);

        // Graphics queue
        if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT) {
            m_queueFamilyIndices.graphics = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);

            VkDeviceQueueCreateInfo queueInfo = vk::initializers::deviceQueueCreateInfo();
            queueInfo.queueFamilyIndex = m_queueFamilyIndices.graphics;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &defaultQueuePriority;
            queueCreateInfos.push_back(queueInfo);
        } else {
            m_queueFamilyIndices.graphics = VK_NULL_HANDLE;
        }

        // Dedicated compute queue
        if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT) {
            m_queueFamilyIndices.compute = getQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);

            if (m_queueFamilyIndices.compute != m_queueFamilyIndices.graphics) {
                // If compute family index differs, we need an additional queue create info for the compute queue
                VkDeviceQueueCreateInfo queueInfo = vk::initializers::deviceQueueCreateInfo();
                queueInfo.queueFamilyIndex = m_queueFamilyIndices.compute;
                queueInfo.queueCount = 1;
                queueInfo.pQueuePriorities = &defaultQueuePriority;
                queueCreateInfos.push_back(queueInfo);
            }
        } else {
            // Else we use the same queue
            m_queueFamilyIndices.compute = m_queueFamilyIndices.graphics;
        }

        // Dedicated transfer queue
        if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT) {
            m_queueFamilyIndices.transfer = getQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);

            if ((m_queueFamilyIndices.transfer != m_queueFamilyIndices.graphics)
                    && (m_queueFamilyIndices.transfer != m_queueFamilyIndices.compute)) {
                // If compute family index differs, we need an additional queue create info for the compute queue
                VkDeviceQueueCreateInfo queueInfo = vk::initializers::deviceQueueCreateInfo();
                queueInfo.queueFamilyIndex = m_queueFamilyIndices.transfer;
                queueInfo.queueCount = 1;
                queueInfo.pQueuePriorities = &defaultQueuePriority;
                queueCreateInfos.push_back(queueInfo);
            }
        } else {
            // Else we use the same queue
            m_queueFamilyIndices.transfer = m_queueFamilyIndices.graphics;
        }

        VkDeviceCreateInfo deviceCreateInfo = vk::initializers::deviceCreateInfo();
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.enabledLayerCount = enableValidationLayers ? static_cast<uint32_t>(validationLayers.size()) : 0;
        deviceCreateInfo.ppEnabledLayerNames = enableValidationLayers ? validationLayers.data() : nullptr;
        deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

        if (!enabledExtensions.empty()) {
            for (const char* enabledExtension : enabledExtensions) {
                if (!extensionSupported(enabledExtension)) {
                    spdlog::warn("Enabled device extension \"" + std::string(enabledExtension) + "\" is not present at device level\n");
                }
            }

            deviceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
            deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
        }

        m_enabledFeatures = enabledFeatures;

        VkResult result = vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_logicalDevice);

        if (result != VK_SUCCESS) {
            return result;
        }

        // Create a default command pool for graphics command buffers
        m_commandPool = createCommandPool(m_queueFamilyIndices.graphics);

        return result;
    }

    VkCommandPool Device::createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags) const {
        VkCommandPoolCreateInfo cmdPoolInfo = vk::initializers::commandPoolCreateInfo();
        cmdPoolInfo.queueFamilyIndex = queueFamilyIndex;
        cmdPoolInfo.flags = createFlags;

        VkCommandPool cmdPool;
        vk::tools::validation(vkCreateCommandPool(m_logicalDevice, &cmdPoolInfo, nullptr, &cmdPool),
                              "Failed to create command pool");

        return cmdPool;
    }

    VkCommandBuffer Device::createCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin) const {
        VkCommandBufferAllocateInfo cmdBufAllocateInfo = vk::initializers::commandBufferAllocateInfo(pool, level, 1);
        VkCommandBuffer cmdBuffer;

        vk::tools::validation(vkAllocateCommandBuffers(m_logicalDevice, &cmdBufAllocateInfo, &cmdBuffer),
                              "Failed to allocate command buffer");

        // If requested, also start recording for the new command buffer
        if (begin) {
            VkCommandBufferBeginInfo cmdBufInfo = vk::initializers::commandBufferBeginInfo();

            vk::tools::validation(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo),
                                  "Failed to begin recording command buffer");
        }

        return cmdBuffer;
    }

    VkCommandBuffer Device::createCommandBuffer(VkCommandBufferLevel level, bool begin) const {
        return createCommandBuffer(level, m_commandPool, begin);;
    }

    void Device::flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free) const {
        if (commandBuffer == VK_NULL_HANDLE) return;

        vk::tools::validation(vkEndCommandBuffer(commandBuffer),
                              "Failed to end command buffer");

        VkSubmitInfo submitInfo = vk::initializers::submitInfo();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        // Create fence to ensure that the command buffer has finished executing
        VkFenceCreateInfo fenceInfo = vk::initializers::fenceCreateInfo();

        VkFence fence;
        vk::tools::validation(vkCreateFence(m_logicalDevice, &fenceInfo, nullptr, &fence),
                              "Failed to create fence");

        // Submit to the queue
        vk::tools::validation(vkQueueSubmit(queue, 1, &submitInfo, fence),
                              "Failed to summit queue");

        // Wait for the fence to signal that command buffer has finished executing
        vk::tools::validation(vkWaitForFences(m_logicalDevice, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max()),
                              "Failed to wait for fence");

        vkDestroyFence(m_logicalDevice, fence, nullptr);

        if (free) vkFreeCommandBuffers(m_logicalDevice, pool, 1, &commandBuffer);
    }

    void Device::flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free) const {
        return flushCommandBuffer(commandBuffer, queue, m_commandPool, free);
    }

    VkResult Device::createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags,
                                  vk::Buffer *buffer, VkDeviceSize size, void *data) const {
        buffer->m_device = m_logicalDevice;

        // Create the buffer handle
        VkBufferCreateInfo bufferCreateInfo = vk::initializers::bufferCreateInfo(usageFlags, size);
        vk::tools::validation(vkCreateBuffer(m_logicalDevice, &bufferCreateInfo, nullptr, &buffer->m_buffer),
                              "Failed to create buffer");

        // Create the memory backing up the buffer handle
        VkMemoryRequirements memReqs;
        vkGetBufferMemoryRequirements(m_logicalDevice, buffer->m_buffer, &memReqs);

        VkMemoryAllocateInfo memAlloc = vk::initializers::memoryAllocateInfo();
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);

        VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};

        if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
            allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
            allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
            memAlloc.pNext = &allocFlagsInfo;
        }

        vk::tools::validation(vkAllocateMemory(m_logicalDevice, &memAlloc, nullptr, &buffer->m_memory),
                              "Failed to allocate buffer memory");

        buffer->m_alignment = memReqs.alignment;
        buffer->m_size = size;
        buffer->m_usageFlags = usageFlags;
        buffer->m_memoryPropertyFlags = memoryPropertyFlags;

        // If a pointer to the buffer data has been passed, map the buffer and copy over the data
        if (data != nullptr) {
            vk::tools::validation(buffer->map(),
                                  "Failed to map buffer");

            memcpy(buffer->m_mapped, data, size);

            if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
                buffer->flush();

            buffer->unmap();
        }

        // Initialize a default descriptor that covers the whole buffer size
        buffer->setupDescriptor();

        // Attach the memory to the buffer object
        return buffer->bind();
    }

    void Device::copyBuffer(vk::Buffer *src, vk::Buffer *dst, VkQueue queue, VkBufferCopy *copyRegion) const {
        VkCommandBuffer copyCmd = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
        VkBufferCopy bufferCopy{};

        if (copyRegion == nullptr) {
            bufferCopy.size = src->m_size;
        } else {
            bufferCopy = *copyRegion;
        }

        vkCmdCopyBuffer(copyCmd, src->m_buffer, dst->m_buffer, 1, &bufferCopy);

        flushCommandBuffer(copyCmd, queue);
    }

} // End namespace vk
