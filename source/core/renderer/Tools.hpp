#ifndef PROTOTYPE_ACTION_RPG_TOOLS_HPP
#define PROTOTYPE_ACTION_RPG_TOOLS_HPP


#include <optional>
#include <string>
#include <vector>
#include <iostream>

#include "vulkan/vulkan.h"
#include "spdlog/spdlog.h"

#include "../Utilities.hpp"


#define VK_CHECK_RESULT(f) { \
	VkResult res = (f);																					\
                                                                                                        \
	if (res != VK_SUCCESS) {																			\
	    spdlog::error("VkResult is \"{}\" in {} at line {} \n", vkc::tools::errorString(res), __FILE__, __LINE__); \
		assert(res == VK_SUCCESS);																		\
	}																									\
}


namespace vkc {

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    namespace tools {

        std::string errorString(VkResult errorCode);

        std::vector<const char*> getRequiredExtensions();

        bool isDeviceSuitable(const VkPhysicalDevice& device, const VkSurfaceKHR& surface, const std::vector<const char *>& enabledExtensions);

        bool checkDeviceExtensionSupport(const VkPhysicalDevice& device, const std::vector<const char *>& deviceExtensions);

        SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

        VkExtent2D chooseSwapExtend(const uint32_t& width, const uint32_t& height, const VkSurfaceCapabilitiesKHR& capabilities);

        VkShaderModule loadShader(const std::vector<char> &code, VkDevice device);

        VkFormat findSupportedFormat(VkPhysicalDevice device, const std::vector<VkFormat>& candidates,
                                     VkImageTiling tiling, VkFormatFeatureFlags features);

    } // namespace tools

} // namespace vk


#endif //PROTOTYPE_ACTION_RPG_TOOLS_HPP
