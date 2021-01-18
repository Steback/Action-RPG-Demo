#include <fstream>

#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"
#include "spdlog/spdlog.h"


#include "Debug.hpp"


namespace vk {

    static void resultValidation(VkResult result, const std::string& errorMessage) {
        if (result != VK_SUCCESS) {
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

    static std::vector<char> readFile(const std::string& fileName) {
        std::ifstream file(fileName, std::ios::ate | std::ios::binary);

        if (!file.is_open()) spdlog::throw_spdlog_ex("Failed to open file: " + fileName);

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

} // End namespace core
