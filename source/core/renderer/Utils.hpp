#include "vulkan/vulkan.hpp"
#include "GLFW/glfw3.h"
#include "spdlog/spdlog.h"


namespace core {

    static void resultValidation(vk::Result result, const std::string& errorMessage) {
        if (result != vk::Result::eSuccess) {
            spdlog::throw_spdlog_ex("[Renderer] " + errorMessage);
        }
    }

    static std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

} // End namespace core
