#include "Instance.hpp"

#include <set>
#include <map>

#include "spdlog/spdlog.h"
#include "fmt/format.h"

#include "../Utilities.hpp"


#ifdef CORE_DEBUG
PFN_vkCreateDebugUtilsMessengerEXT  pfnVkCreateDebugUtilsMessengerEXT;
PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT;

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(VkInstance instance,
                                                              const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkDebugUtilsMessengerEXT *pMessenger) {
    return pfnVkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance instance,
                                                           VkDebugUtilsMessengerEXT messenger,
                                                           VkAllocationCallbacks const *pAllocator) {
    return pfnVkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugMessageFunc(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                VkDebugUtilsMessengerCallbackDataEXT const *pCallbackData,
                                                void* pUserData) {
    auto* instance = reinterpret_cast<core::Instance*>(pUserData);
    std::unordered_map<uint32_t, std::string>& debugMessages = instance->m_debugMessages;

    if (debugMessages.find(pCallbackData->messageIdNumber) == debugMessages.end()) {
        std::string message{};
        message += vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(messageTypes)) + ":\n";
        message += std::string("\t") + "messageIDName   = <" + pCallbackData->pMessageIdName + ">\n";
        message += std::string("\t") + "messageIdNumber = " + std::to_string(pCallbackData->messageIdNumber) + "\n";
        message += std::string("\t") + "message         = <" + pCallbackData->pMessage + ">\n";

        if (0 < pCallbackData->queueLabelCount) {
            message += std::string("\t") + "Queue Labels:\n";

            for (uint8_t i = 0; i < pCallbackData->queueLabelCount; i++) {
                message += std::string("\t\t") + "labelName = <" + pCallbackData->pQueueLabels[i].pLabelName + ">\n";
            }
        }
        if (0 < pCallbackData->cmdBufLabelCount) {
            message += std::string("\t") + "CommandBuffer Labels:\n";

            for ( uint8_t i = 0; i < pCallbackData->cmdBufLabelCount; i++ ) {
                message += std::string("\t\t") + "labelName = <" + pCallbackData->pCmdBufLabels[i].pLabelName + ">\n";
            }
        }

        if (0 < pCallbackData->objectCount) {
            message += std::string("\t") + "Objects:\n";

            for ( uint8_t i = 0; i < pCallbackData->objectCount; i++ ) {
                message += std::string("\t\t") + "Object " + std::to_string(i) + "\n";
                message += std::string("\t\t\t")
                           + "objectType   = "
                           + vk::to_string( static_cast<vk::ObjectType>( pCallbackData->pObjects[i].objectType ) ) + "\n";
                message += std::string("\t\t\t")
                           + "objectHandle = " + std::to_string(pCallbackData->pObjects[i].objectHandle) + "\n";

                if (pCallbackData->pObjects[i].pObjectName) {
                    message += std::string("\t\t\t") + "objectName   = <" + pCallbackData->pObjects[i].pObjectName + ">\n";
                }
            }
        }

        debugMessages[pCallbackData->messageIdNumber] = pCallbackData->pMessage;

        spdlog::error(message);
    }

    return false;
}
#endif

std::vector<const char*> getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef CORE_DEBUG
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    return extensions;
}

namespace core {

    Instance::Instance() = default;

    Instance::Instance(const vk::ApplicationInfo& appInfo) {
        std::vector<const char*> reqExtensions = getRequiredExtensions();
        std::vector<const char*> reqLayer;

        reqExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

#ifdef CORE_DEBUG
        reqLayer.push_back("VK_LAYER_KHRONOS_validation");
#endif

        vk::InstanceCreateInfo createInfo{
                .pApplicationInfo = &appInfo,
                .enabledLayerCount = static_cast<uint32_t>(reqLayer.size()),
                .ppEnabledLayerNames = reqLayer.data(),
                .enabledExtensionCount = static_cast<uint32_t>(reqExtensions.size()),
                .ppEnabledExtensionNames = reqExtensions.data()
        };

#ifdef CORE_DEBUG
        spdlog::info("[Debug] Setup");

        vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo{
                .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
                .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                               vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
                .pfnUserCallback = &debugMessageFunc,
                .pUserData = this
        };

        createInfo.pNext = &debugCreateInfo;

        m_instance = vk::createInstance(createInfo);

        pfnVkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(m_instance.getProcAddr("vkCreateDebugUtilsMessengerEXT"));

        if (!pfnVkCreateDebugUtilsMessengerEXT)
            throw std::runtime_error("GetInstanceProcAddr: Unable to find pfnVkCreateDebugUtilsMessengerEXT function.");

        pfnVkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(m_instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT"));

        if (!pfnVkDestroyDebugUtilsMessengerEXT)
            throw std::runtime_error("GetInstanceProcAddr: Unable to find pfnVkDestroyDebugUtilsMessengerEXT function.");

        m_debugMessenger = m_instance.createDebugUtilsMessengerEXT(debugCreateInfo);
#else
        m_instance = vk::createInstance(createInfo);
#endif
    }

    Instance::~Instance() = default;

    void Instance::destroy() {
#ifdef CORE_DEBUG
        m_instance.destroyDebugUtilsMessengerEXT(m_debugMessenger);
#endif

        m_instance.destroy();
    }

    vk::Instance Instance::getInstance() const {
        return m_instance;
    }

    vk::PhysicalDevice Instance::selectPhysicalDevice(const std::vector<const char *>& enabledExtensions) {
        auto physicalDevices = m_instance.enumeratePhysicalDevices();

        if (physicalDevices.empty()) throw  std::runtime_error("Failed to find GPUs with Vulkan support");

        for (const auto& device : physicalDevices) {
            std::set<std::string> reqExtension(enabledExtensions.begin(), enabledExtensions.end());

            for (auto& extension : device.enumerateDeviceExtensionProperties()) {
                reqExtension.erase(extension.extensionName);
            }

            if (reqExtension.empty()) return device;
        }

        throw std::runtime_error("Failed to find a suitable GPU!");
    }

    vk::SurfaceKHR Instance::createSurface(GLFWwindow* window) {
        VkSurfaceKHR surface;
        glfwCreateWindowSurface(m_instance, window, nullptr, &surface);

        return vk::SurfaceKHR(surface);
    }

    void Instance::destroy(vk::SurfaceKHR surface) {
        m_instance.destroy(surface);
    }

} // End namespace vk