#include "Debug.hpp"

#include <utility>

#include "spdlog/spdlog.h"

#include "Utils.hpp"


namespace core {

    Debug::Debug() = default;

    Debug::Debug(std::vector<const char *> layerNames)
            : mValidationLayers(std::move(layerNames)) {

    }

    Debug::~Debug() = default;

    void Debug::init(vk::Instance& instance) {
        VkDebugUtilsMessengerCreateInfoEXT createInfo;

        populateDebugMessengerCreateInfo(createInfo);

        if (createDebugUtilsMessengerEXT(instance.operator VkInstance_T *(), &createInfo, nullptr, &mDebugMessenger)
                != VK_SUCCESS) {
            spdlog::throw_spdlog_ex("Failed to set up debug messenger!");
        }
    }

    bool Debug::checkValidationLayerSupport() {
        spdlog::info("[Validation-Layer] Check support");

        uint32_t layerCount;
        vk::enumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<vk::LayerProperties> availableLayers(layerCount);
        vk::enumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : mValidationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (std::strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    uint32_t Debug::getValidationLayersCount() {
        return static_cast<uint32_t>(mValidationLayers.size());
    }

    const char **Debug::getValidationLayers() {
        return mValidationLayers.data();
    }

    void Debug::clean(vk::Instance& instance) {
        destroyDebugUtilsMessengerEXT(instance.operator VkInstance_T *(), mDebugMessenger, nullptr);
    }

    VkBool32 Debug::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                  VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                  VkDebugUtilsMessengerCallbackDataEXT const *pCallbackData,
                                  void *userData ) {
        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            spdlog::error("[Validation-Layer] {}", pCallbackData->pMessage);
        }

        return VK_FALSE;
    }

    VkResult Debug::createDebugUtilsMessengerEXT(VkInstance instance,
                                        VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                        VkAllocationCallbacks *pAllocator,
                                        VkDebugUtilsMessengerEXT *pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,
                                                                               "vkCreateDebugUtilsMessengerEXT");

        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void Debug::destroyDebugUtilsMessengerEXT(VkInstance instance,
                                              VkDebugUtilsMessengerEXT debugMessenger,
                                              const VkAllocationCallbacks *pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,
                                                                                "vkDestroyDebugUtilsMessengerEXT");

        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    void Debug::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    void Debug::populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT &createInfo) {
        createInfo = {
                .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
                .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
                .pfnUserCallback = debugCallback
        };
    }


} // End namespace core