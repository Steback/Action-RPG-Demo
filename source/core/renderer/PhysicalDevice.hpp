#ifndef PROTOTYPE_ACTION_RPG_PHYSICALDEVICE_HPP
#define PROTOTYPE_ACTION_RPG_PHYSICALDEVICE_HPP


#include <optional>
#include <string>
#include <vector>

#include "vulkan/vulkan.h"


namespace vk {

    const std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    struct PhysicalDevice {
        VkPhysicalDevice device = VK_NULL_HANDLE;
        std::string deiceName{};
    };

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphics;
        std::optional<uint32_t> present;

        [[nodiscard]] bool isComplete() const;
    };

    bool isDeviceSuitable(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);

    VkPhysicalDeviceProperties getPhysicalDeviceProperties(const VkPhysicalDevice& device);

    VkPhysicalDeviceFeatures getPhysicalDeviceFeatures(const VkPhysicalDevice& device);

    QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);

    bool checkDeviceExtensionSupport(const VkPhysicalDevice& device, const std::vector<const char *>& deviceExtensions);

} // End namespace vk


#endif //PROTOTYPE_ACTION_RPG_PHYSICALDEVICE_HPP
