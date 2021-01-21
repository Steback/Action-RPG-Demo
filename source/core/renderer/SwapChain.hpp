#ifndef PROTOTYPE_ACTION_RPG_SWAPCHAIN_HPP
#define PROTOTYPE_ACTION_RPG_SWAPCHAIN_HPP


#include <vector>

#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"


namespace vk {
    struct SwapChain {
        VkSwapchainKHR mSwapChain{};
        std::vector<VkImage> mImages;
        std::vector<VkImageView> mImageViews;
        std::vector<VkFramebuffer> mFramebuffers;
        VkFormat mImageFormat{};
        VkExtent2D mExtent{};
    };


    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    VkExtent2D chooseSwapExtend(GLFWwindow *window, const VkSurfaceCapabilitiesKHR& capabilities);

} // End namespace vk


#endif //PROTOTYPE_ACTION_RPG_SWAPCHAIN_HPP
