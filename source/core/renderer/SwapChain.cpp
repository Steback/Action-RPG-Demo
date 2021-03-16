#include "SwapChain.hpp"

#include "Tools.hpp"
#include "Initializers.hpp"


namespace vk {

    SwapChain::SwapChain() = default;

    SwapChain::~SwapChain() = default;

    void SwapChain::connect(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface) {
        m_physicalDevice = physicalDevice,
        m_device = device;
        m_surface = surface;
    }

    void SwapChain::create(uint32_t& width, uint32_t& height, uint32_t graphicsFamilyIndex, uint32_t presetFamilyIndex) {
        vk::SwapChainSupportDetails swapChainSupport = vk::tools::querySwapChainSupport(m_physicalDevice, m_surface);

        VkSurfaceFormatKHR surfaceFormat = vk::tools::chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = vk::tools::chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = vk::tools::chooseSwapExtend(width, height, swapChainSupport.capabilities);

        m_imageCount = swapChainSupport.capabilities.minImageCount + 1;

        if (swapChainSupport.capabilities.maxImageCount > 0 &&
                m_imageCount > swapChainSupport.capabilities.maxImageCount) {
            m_imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo = vk::initializers::swapchainCreateInfo();
        createInfo.surface = m_surface;
        createInfo.minImageCount = m_imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = nullptr;

        std::vector<uint32_t> queueFamilyIndices = {
                graphicsFamilyIndex, presetFamilyIndex
        };

        if (graphicsFamilyIndex != presetFamilyIndex) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT,
                    createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
            createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
                    createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        VK_CHECK_RESULT(vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain));

        vkGetSwapchainImagesKHR(m_device, m_swapChain, &m_imageCount, nullptr);
        m_images.resize(m_imageCount);
        vkGetSwapchainImagesKHR(m_device, m_swapChain, &m_imageCount, m_images.data());

        m_extent = extent;
        m_format = surfaceFormat.format;
        m_colorSpace = surfaceFormat.colorSpace;

        m_buffers.resize(m_images.size());

        for (size_t i = 0; i < m_imageCount; ++i) {
            VkImageViewCreateInfo colorAttachmentView = initializers::imageViewCreateInfo();
            colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
            colorAttachmentView.format = m_format;
            colorAttachmentView.components = {
                    VK_COMPONENT_SWIZZLE_R,
                    VK_COMPONENT_SWIZZLE_G,
                    VK_COMPONENT_SWIZZLE_B,
                    VK_COMPONENT_SWIZZLE_A
            };
            colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            colorAttachmentView.subresourceRange.baseMipLevel = 0;
            colorAttachmentView.subresourceRange.levelCount = 1;
            colorAttachmentView.subresourceRange.baseArrayLayer = 0;
            colorAttachmentView.subresourceRange.layerCount = 1;
            colorAttachmentView.flags = 0;

            m_buffers[i].image = m_images[i];

            colorAttachmentView.image = m_buffers[i].image;

            VK_CHECK_RESULT(vkCreateImageView(m_device, &colorAttachmentView, nullptr, &m_buffers[i].view));
        }
    }

    VkResult SwapChain::acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t *imageIndex) {
        return vkAcquireNextImageKHR(m_device, m_swapChain, std::numeric_limits<uint64_t>::max(), presentCompleteSemaphore,
                                     VK_NULL_HANDLE, imageIndex);
    }

    VkResult SwapChain::queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore) {
        VkPresentInfoKHR presentInfo = vk::initializers::presentInfo();
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_swapChain;
        presentInfo.pImageIndices = &imageIndex;

        // Check if a wait semaphore has been specified to wait for before presenting the image
        if (waitSemaphore != VK_NULL_HANDLE) {
            presentInfo.pWaitSemaphores = &waitSemaphore;
            presentInfo.waitSemaphoreCount = 1;
        }

        return vkQueuePresentKHR(queue, &presentInfo);
    }

    void SwapChain::cleanup() {
        if (m_swapChain != VK_NULL_HANDLE) {
            for (uint32_t i = 0; i < m_imageCount; ++i) {
                vkDestroyImageView(m_device, m_buffers[i].view, nullptr);
            }
        }

        vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
        m_swapChain = VK_NULL_HANDLE;
    }

    VkSwapchainKHR SwapChain::getSwapChain() const {
        return m_swapChain;
    }

    VkFormat SwapChain::getFormat() const {
        return m_format;
    }

    VkExtent2D SwapChain::getExtent() const {
        return m_extent;
    }

    uint32_t SwapChain::getImageCount() const {
        return m_imageCount;
    }

    VkImageView SwapChain::getImageView(size_t index) const {
        return m_buffers[index].view;
    }

} // End namespace vk
