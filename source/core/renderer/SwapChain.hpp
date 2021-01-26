#ifndef PROTOTYPE_ACTION_RPG_SWAPCHAIN_HPP
#define PROTOTYPE_ACTION_RPG_SWAPCHAIN_HPP


#include <vector>

#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"

#include "../window/Window.hpp"


namespace vk {

    struct SwapChain {
        VkSwapchainKHR swapchain{};
        std::vector<VkImage> images;
        std::vector<VkImageView> imageViews;
        std::vector<VkFramebuffer> framebuffers;
        VkFormat format{};
        VkExtent2D extent{};
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    VkExtent2D chooseSwapExtend(const core::WindowSize& windowSize, const VkSurfaceCapabilitiesKHR& capabilities);

} // End namespace vk


#endif //PROTOTYPE_ACTION_RPG_SWAPCHAIN_HPP
