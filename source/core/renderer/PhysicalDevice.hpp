#ifndef PROTOTYPE_ACTION_RPG_PHYSICALDEVICE_HPP
#define PROTOTYPE_ACTION_RPG_PHYSICALDEVICE_HPP


#include <optional>
#include <string>

#include "vulkan/vulkan.h"


namespace vk {

    struct PhysicalDevice {
        VkPhysicalDevice device = VK_NULL_HANDLE;
        std::string deiceName{};
    };

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        [[nodiscard]] bool isComplete() const;
    };

    bool isDeviceSuitable(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);

    VkPhysicalDeviceProperties getPhysicalDeviceProperties(const VkPhysicalDevice& device);

    VkPhysicalDeviceFeatures getPhysicalDeviceFeatures(const VkPhysicalDevice& device);

    QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);

} // End namespace vk


#endif //PROTOTYPE_ACTION_RPG_PHYSICALDEVICE_HPP
