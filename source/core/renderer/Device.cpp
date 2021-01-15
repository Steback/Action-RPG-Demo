#include "Device.hpp"

#include <set>

#include "Debug.hpp"
#include "Utils.hpp"


namespace vk {

    Device::Device() = default;

    Device::~Device() = default;

    void Device::init(const PhysicalDevice& physicalDevice, VkSurfaceKHR& surface) {
        mPhysicalDevice = physicalDevice;
        QueueFamilyIndices indices = findQueueFamilies(mPhysicalDevice.device, surface);
        VkPhysicalDeviceFeatures physicalDeviceFeatures{};
        std::set<uint32_t> uniqueQueueFamilies = {
                indices.graphicsFamily.value(),
                indices.presentFamily.value() };

        float queuePriority = 1.0f;
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        queueCreateInfos.reserve(uniqueQueueFamilies.size());

        for (uint32_t queueFamily : uniqueQueueFamilies) {
            queueCreateInfos.push_back(VkDeviceQueueCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = queueFamily,
                .queueCount = 1,
                .pQueuePriorities = &queuePriority });
        }

        VkDeviceCreateInfo deviceCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
                .pQueueCreateInfos = queueCreateInfos.data(),
                .enabledLayerCount = enableValidationLayers ? static_cast<uint32_t>(validationLayers.size()) : 0,
                .ppEnabledLayerNames = enableValidationLayers ? validationLayers.data() : nullptr,
                .enabledExtensionCount = 0,
                .ppEnabledExtensionNames = nullptr,
                .pEnabledFeatures = &physicalDeviceFeatures,
        };

        resultValidation(vkCreateDevice(physicalDevice.device, &deviceCreateInfo, nullptr, &mDevice),
                         "Failed to create logical device");

        vkGetDeviceQueue(mDevice, indices.graphicsFamily.value(), 0, &mGraphicsQueue);
        vkGetDeviceQueue(mDevice, indices.presentFamily.value(), 0, &mPresentQueue);
    }

    void Device::destroy() {
        vkDestroyDevice(mDevice, nullptr);
    }

} // End namespace vk
