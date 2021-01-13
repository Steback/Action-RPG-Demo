#include "Instance.hpp"


namespace vk {

    Instance::Instance() = default;

    Instance::~Instance() = default;

    void Instance::init(VkApplicationInfo& appInfo) {
        std::vector<const char*> extensions = vk::getRequiredExtensions();
        std::vector<const char*> validationLayers = {
                "VK_LAYER_KHRONOS_validation"
        };

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;

        if (enableValidationLayers) populateDebugMessengerCreateInfo(debugCreateInfo);

        VkInstanceCreateInfo createInfo{
                .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                .pNext = enableValidationLayers ? &debugCreateInfo : nullptr,
                .pApplicationInfo = &appInfo,
                .enabledLayerCount = enableValidationLayers ? static_cast<uint32_t>(validationLayers.size()) : 0,
                .ppEnabledLayerNames = enableValidationLayers ? validationLayers.data() : nullptr,
                .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
                .ppEnabledExtensionNames = extensions.data(),
        };

        vk::resultValidation(vkCreateInstance(&createInfo, nullptr, &mInstance),
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

    void Instance::setupDebugMessenger() {
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        resultValidation(createDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessenger),
                         "Failed to set up debug messenger");
    }

} // End namespace vk