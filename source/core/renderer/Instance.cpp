#include "Instance.hpp"


namespace vk {

    Instance::Instance() = default;

    Instance::~Instance() = default;

    void Instance::init(VkApplicationInfo& appInfo) {
        std::vector<const char*> extensions = vk::getRequiredExtensions();

        VkInstanceCreateInfo createInfo{
                .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                .pApplicationInfo = &appInfo,
                .enabledLayerCount = enableValidationLayers ? static_cast<uint32_t>(validationLayers.size()) : 0,
                .ppEnabledLayerNames = enableValidationLayers ? validationLayers.data() : nullptr,
                .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
                .ppEnabledExtensionNames = extensions.data(),
        };

        if (enableValidationLayers) {
            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
            populateDebugMessengerCreateInfo(debugCreateInfo);

            createInfo.pNext = &debugCreateInfo;
        }

        resultValidation(vkCreateInstance(&createInfo, nullptr, &mInstance),
                             "Failed to create instance");

        if (enableValidationLayers) {
            if (!checkValidationLayerSupport(validationLayers))
                spdlog::throw_spdlog_ex("[Renderer] Validation layers requested, but not available");

            setupDebugMessenger();
        }
    }

    void Instance::destroy() {
        if (enableValidationLayers) {
            destroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
        }

        vkDestroyInstance(mInstance, nullptr);
    }

    VkInstance &Instance::operator*() {
        return mInstance;
    }

    void Instance::pickPhysicalDevice(PhysicalDevice &physicalDevice, VkSurfaceKHR& surface) {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);

        if (deviceCount == 0) spdlog::throw_spdlog_ex("Failed to find GPUs with Vulkan support");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

        for (const auto& device : devices) {
            if (isDeviceSuitable(device, surface)) {
                physicalDevice.device = device;
                physicalDevice.deiceName = getPhysicalDeviceProperties(device).deviceName;
                break;
            }
        }

        if (physicalDevice.device == VK_NULL_HANDLE) spdlog::throw_spdlog_ex("Failed to find a suitable GPU!");
    }

    void Instance::createSurface(GLFWwindow* window, VkSurfaceKHR& surface) {
        resultValidation(glfwCreateWindowSurface(mInstance, window, nullptr, &surface),
                         "Failed to create window surface");
    }

    void Instance::destroySurface(VkSurfaceKHR &surface) {
        vkDestroySurfaceKHR(mInstance, surface, nullptr);
    }

    void Instance::setupDebugMessenger() {
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        resultValidation(createDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessenger),
                         "Failed to set up debug messenger");
    }

} // End namespace vk