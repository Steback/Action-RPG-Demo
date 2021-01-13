#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"
#include "spdlog/spdlog.h"


namespace vk {

    static void resultValidation(VkResult result, const std::string& errorMessage) {
        if (result != VK_SUCCESS) {
            spdlog::throw_spdlog_ex("[Renderer] " + errorMessage);
        }
    }

} // End namespace core
