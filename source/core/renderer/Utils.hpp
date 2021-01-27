#ifndef PROTOTYPE_ACTION_RPG_UTILS_HPP
#define PROTOTYPE_ACTION_RPG_UTILS_HPP


#include <fstream>

#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"
#include "spdlog/spdlog.h"

#include "Debug.hpp"
#include "../Utilities.hpp"


namespace vk {

    inline void validation(VkResult result, const std::string& error) {
        if (result != VK_SUCCESS) core::throw_ex(error);
    }

    inline std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    inline std::vector<char> readFile(const std::string& fileName) {
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


#endif //PROTOTYPE_ACTION_RPG_UTILS_HPP
