#include "Instance.hpp"

#include "Tools.hpp"


namespace vk {

    Instance::Instance() = default;

    Instance::~Instance() = default;

    void Instance::init(VkApplicationInfo& appInfo) {
        std::vector<const char*> extensions = vk::tools::getRequiredExtensions();

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

        vk::tools::validation(vkCreateInstance(&createInfo, nullptr, &m_instance),
                              "Failed to create instance");

        if (enableValidationLayers) {
            if (!checkValidationLayerSupport(validationLayers))
                spdlog::throw_spdlog_ex("[Renderer] Validation layers requested, but not available");

            setupDebugMessenger();
        }
    }

    void Instance::destroy() {
        if (enableValidationLayers) {
            destroyDebugUtilsMessengerEXT(m_instance, debugMessenger, nullptr);
        }

        vkDestroyInstance(m_instance, nullptr);
    }

    VkInstance &Instance::operator*() {
        return m_instance;
    }

    void Instance::pickPhysicalDevice(VkPhysicalDevice &physicalDevice, VkSurfaceKHR& surface,
                                      const std::vector<const char *>& enabledExtensions) {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

        if (deviceCount == 0) core::throw_ex("Failed to find GPUs with Vulkan support");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

        for (const auto& device : devices) {
            if (vk::tools::isDeviceSuitable(device, surface, enabledExtensions)) {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) core::throw_ex("Failed to find a suitable GPU!");
    }

    void Instance::createSurface(GLFWwindow* window, VkSurfaceKHR& surface) {
        vk::tools::validation(glfwCreateWindowSurface(m_instance, window, nullptr, &surface),
                              "Failed to create window surface");
    }

    void Instance::destroySurface(VkSurfaceKHR &surface) {
        vkDestroySurfaceKHR(m_instance, surface, nullptr);
    }

    void Instance::setupDebugMessenger() {
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        vk::tools::validation(createDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &debugMessenger),
                              "Failed to set up debug messenger");
    }

} // End namespace vk