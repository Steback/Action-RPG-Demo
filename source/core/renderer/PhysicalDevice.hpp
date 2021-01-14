#ifndef PROTOTYPE_ACTION_RPG_PHYSICALDEVICE_HPP
#define PROTOTYPE_ACTION_RPG_PHYSICALDEVICE_HPP


#include <optional>

#include "vulkan/vulkan.h"


namespace vk {

    struct PhysicalDevice {
        VkPhysicalDevice device = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties properties{};
    };

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;

        [[nodiscard]] bool isComplete() const;
    };

    bool isDeviceSuitable(const VkPhysicalDevice& device);

    VkPhysicalDeviceProperties getPhysicalDeviceProperties(const VkPhysicalDevice& device);

    VkPhysicalDeviceFeatures getPhysicalDeviceFeatures(const VkPhysicalDevice& device);

    QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& device);

} // End namespace vk


#endif //PROTOTYPE_ACTION_RPG_PHYSICALDEVICE_HPP
