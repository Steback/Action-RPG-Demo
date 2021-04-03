#ifndef PROTOTYPE_ACTION_RPG_SWAPCHAIN_HPP
#define PROTOTYPE_ACTION_RPG_SWAPCHAIN_HPP


#include <vector>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "vulkan/vulkan.hpp"
#include "GLFW/glfw3.h"

#include "../window/Window.hpp"


namespace vkc {
    class Device;

    struct SwapChainSupportDetails {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

    struct SwapChainBuffer {
        vk::Image image;
        vk::ImageView view;
    };

    class SwapChain {
    public:
        SwapChain();

        ~SwapChain();

        void connect(const std::shared_ptr<vkc::Device>& device, vk::SurfaceKHR surface);

        void create(uint32_t& width, uint32_t& height, uint32_t graphicsFamilyIndex, uint32_t presetFamilyIndex);

        vk::Result acquireNextImage(vk::Semaphore presentCompleteSemaphore, uint32_t* imageIndex);

        vk::Result queuePresent(vk::Queue queue, uint32_t imageIndex, vk::Semaphore waitSemaphore = nullptr);

        void cleanup();

        [[nodiscard]] vk::SwapchainKHR getSwapChain() const;

        [[nodiscard]] vk::Format getFormat() const;

        [[nodiscard]] vk::Extent2D getExtent() const;

        [[nodiscard]] uint32_t getImageCount() const;

        [[nodiscard]] vk::ImageView getImageView(size_t index) const;

        vk::SurfaceKHR getSurface();

    private:
        vk::Format m_format{};
        vk::Extent2D m_extent{};
        vk::ColorSpaceKHR m_colorSpace{};
        vk::SwapchainKHR m_swapChain = nullptr;
        uint32_t m_imageCount{};
        std::vector<vk::Image> m_images;
        std::vector<SwapChainBuffer> m_buffers;
        vk::Device m_device{};
        vk::PhysicalDevice m_physicalDevice{};
        vk::SurfaceKHR m_surface{};
    };

} // End namespace vk


#endif //PROTOTYPE_ACTION_RPG_SWAPCHAIN_HPP
