#include "Buffer.hpp"

#include "spdlog/spdlog.h"


namespace vk {

    uint32_t findMemoryType(const VkPhysicalDevice& device, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(device, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        spdlog::throw_spdlog_ex("Failed to find suitable memory type");

        return -1;
    }

} // End namespace vk