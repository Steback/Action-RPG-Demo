#include "Device.hpp"

#include "Debug.hpp"
#include "Utils.hpp"


namespace vk {

    Device::Device() = default;

    Device::~Device() = default;

    void Device::init(const PhysicalDevice& physicalDevice) {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice.device);
        VkPhysicalDeviceFeatures physicalDeviceFeatures{};
        float queuePriority = 1.0f;

        VkDeviceQueueCreateInfo queueCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = indices.graphicsFamily.value(),
                .queueCount = 1,
                .pQueuePriorities = &queuePriority
        };

        VkDeviceCreateInfo deviceCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                .queueCreateInfoCount = 1,
                .pQueueCreateInfos = &queueCreateInfo,
                .enabledLayerCount = enableValidationLayers ? static_cast<uint32_t>(validationLayers.size()) : 0,
                .ppEnabledLayerNames = enableValidationLayers ? validationLayers.data() : nullptr,
                .enabledExtensionCount = 0,
                .ppEnabledExtensionNames = nullptr,
                .pEnabledFeatures = &physicalDeviceFeatures,
        };

        resultValidation(vkCreateDevice(physicalDevice.device, &deviceCreateInfo, nullptr, &mDevice),
                         "Failed to create logical device");

        vkGetDeviceQueue(mDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
    }

    void Device::destroy() {
        vkDestroyDevice(mDevice, nullptr);
    }

} // End namespace vk
