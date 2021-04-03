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



    namespace tools {

        std::string errorString(VkResult errorCode);

        std::vector<const char*> getRequiredExtensions();

        VkShaderModule loadShader(const std::vector<char> &code, VkDevice device);

        VkFormat findSupportedFormat(VkPhysicalDevice device, const std::vector<VkFormat>& candidates,
                                     VkImageTiling tiling, VkFormatFeatureFlags features);

    } // namespace tools

} // namespace vk


#endif //PROTOTYPE_ACTION_RPG_TOOLS_HPP
