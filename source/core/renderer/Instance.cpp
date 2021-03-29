#include "Instance.hpp"

#include "Tools.hpp"

#ifdef CORE_DEBUG
#include "Debug.hpp"
#endif

namespace vkc {

    Instance::Instance() = default;

    Instance::~Instance() = default;

    void Instance::init(VkApplicationInfo& appInfo) {
        std::vector<const char*> extensions = vkc::tools::getRequiredExtensions();

        VkInstanceCreateInfo createInfo{
                .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                .pApplicationInfo = &appInfo,
                .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
                .ppEnabledExtensionNames = extensions.data(),
        };

#ifdef CORE_DEBUG
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        populateDebugMessengerCreateInfo(debugCreateInfo);

        createInfo.pNext = &debugCreateInfo;
#endif

        VK_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &m_instance));

#ifdef CORE_DEBUG
        if (!checkValidationLayerSupport(validationLayers))
            throw std::runtime_error("[Renderer] Validation layers requested, but not available");

        setupDebugMessenger();
#endif
    }

    void Instance::destroy() {
#ifdef CORE_DEBUG
        destroyDebugUtilsMessengerEXT(m_instance, debugMessenger, nullptr);
#endif

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
            if (vkc::tools::isDeviceSuitable(device, surface, enabledExtensions)) {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) core::throw_ex("Failed to find a suitable GPU!");
    }

    void Instance::createSurface(GLFWwindow* window, VkSurfaceKHR& surface) {
        VK_CHECK_RESULT(glfwCreateWindowSurface(m_instance, window, nullptr, &surface));
    }

    void Instance::destroySurface(VkSurfaceKHR &surface) {
        vkDestroySurfaceKHR(m_instance, surface, nullptr);
    }

#ifdef CORE_DEBUG
    void Instance::setupDebugMessenger() {
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        VK_CHECK_RESULT(createDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &debugMessenger));
    }
#endif

} // End namespace vk