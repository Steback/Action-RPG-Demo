#ifndef PROTOTYPE_ACTION_RPG_SWAPCHAIN_HPP
#define PROTOTYPE_ACTION_RPG_SWAPCHAIN_HPP


#include <vector>

#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"

#include "../window/Window.hpp"


namespace vkc {

    struct SwapChainBuffer {
        VkImage image;
        VkImageView view;
    };

    class SwapChain {
    public:
        SwapChain();

        ~SwapChain();

        void connect(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface);

        void create(uint32_t& width, uint32_t& height, uint32_t graphicsFamilyIndex, uint32_t presetFamilyIndex);

        VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex);

        VkResult queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);

        void cleanup();

        [[nodiscard]] VkSwapchainKHR getSwapChain() const;

        [[nodiscard]] VkFormat getFormat() const;

        [[nodiscard]] VkExtent2D getExtent() const;

        [[nodiscard]] uint32_t getImageCount() const;

        [[nodiscard]] VkImageView getImageView(size_t index) const;

    private:
        VkFormat m_format{};
        VkExtent2D m_extent{};
        VkColorSpaceKHR m_colorSpace{};
        VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
        uint32_t m_imageCount{};
        std::vector<VkImage> m_images;
        std::vector<SwapChainBuffer> m_buffers;
        VkDevice m_device{};
        VkPhysicalDevice m_physicalDevice{};
        VkSurfaceKHR m_surface{};
    };

} // End namespace vk


#endif //PROTOTYPE_ACTION_RPG_SWAPCHAIN_HPP
