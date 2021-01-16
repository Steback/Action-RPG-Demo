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
                .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
                .ppEnabledExtensionNames = deviceExtensions.data(),
                .pEnabledFeatures = &physicalDeviceFeatures,
        };

        resultValidation(vkCreateDevice(mPhysicalDevice.device, &deviceCreateInfo, nullptr, &mDevice),
                         "Failed to create logical device");

        vkGetDeviceQueue(mDevice, indices.graphicsFamily.value(), 0, &mGraphicsQueue);
        vkGetDeviceQueue(mDevice, indices.presentFamily.value(), 0, &mPresentQueue);
    }

    void Device::destroy() {
        vkDestroyDevice(mDevice, nullptr);
    }

    void Device::createSwapChain(SwapChain &swapChain, GLFWwindow *window, VkSurfaceKHR surface) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(mPhysicalDevice.device, surface);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtend(window, swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = surface,
            .minImageCount = imageCount,
            .imageFormat = surfaceFormat.format,
            .imageColorSpace = surfaceFormat.colorSpace,
            .imageExtent = extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .preTransform = swapChainSupport.capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = presentMode,
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE
        };

        QueueFamilyIndices indices = findQueueFamilies(mPhysicalDevice.device, surface);
        std::vector<uint32_t> queueFamilyIndices = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT,
            createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
            createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        resultValidation(vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &swapChain.mSwapChain),
                         "Failed to create swap chain");

        vkGetSwapchainImagesKHR(mDevice, swapChain.mSwapChain, &imageCount, nullptr);
        swapChain.mImages.resize(imageCount);
        vkGetSwapchainImagesKHR(mDevice, swapChain.mSwapChain, &imageCount, swapChain.mImages.data());

        swapChain.mExtent = extent;
        swapChain.mImageFormat = surfaceFormat.format;
    }

    void Device::destroySwapChain(SwapChain& swapChain) {
        vkDestroySwapchainKHR(mDevice, swapChain.mSwapChain, nullptr);
    }

} // End namespace vk
