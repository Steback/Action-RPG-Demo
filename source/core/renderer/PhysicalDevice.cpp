#include "PhysicalDevice.hpp"

#include <vector>


namespace vk {

    bool QueueFamilyIndices::isComplete() const {
        return graphicsFamily.has_value();
    }

    bool isDeviceSuitable(const VkPhysicalDevice& device) {
       QueueFamilyIndices indices = findQueueFamilies(device);

        return indices.isComplete();
    }

    VkPhysicalDeviceProperties getPhysicalDeviceProperties(const VkPhysicalDevice& device) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        return deviceProperties;
    }

    VkPhysicalDeviceFeatures getPhysicalDeviceFeatures(const VkPhysicalDevice& device) {
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        return deviceFeatures;
    }

    QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& device) {
        QueueFamilyIndices indices{};

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;

        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            if (indices.isComplete()) break;

            i++;
        }

        return indices;
    }
} // End namespace vk
