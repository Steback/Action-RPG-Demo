#ifndef PROTOTYPE_ACTION_RPG_TOOLS_HPP
#define PROTOTYPE_ACTION_RPG_TOOLS_HPP


#include <optional>
#include <string>
#include <vector>

#include "vulkan/vulkan.h"

#include "../Utilities.hpp"


namespace vk {

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    namespace tools {

        inline void validation(VkResult result, const std::string& error) {
            if (result != VK_SUCCESS) core::throw_ex(error);
        }

        std::vector<const char*> getRequiredExtensions();

        bool isDeviceSuitable(const VkPhysicalDevice& device, const VkSurfaceKHR& surface, const std::vector<const char *>& enabledExtensions);

        bool checkDeviceExtensionSupport(const VkPhysicalDevice& device, const std::vector<const char *>& deviceExtensions);

        SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

        VkExtent2D chooseSwapExtend(const uint32_t& width, const uint32_t& height, const VkSurfaceCapabilitiesKHR& capabilities);

        VkShaderModule loadShader(const std::vector<char> &code, VkDevice device);

    } // namespace tools

} // namespace vk


#endif //PROTOTYPE_ACTION_RPG_TOOLS_HPP
