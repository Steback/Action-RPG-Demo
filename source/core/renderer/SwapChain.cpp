#include "SwapChain.hpp"

#include "Tools.hpp"
#include "Initializers.hpp"
#include "Device.hpp"


vkc::SwapChainSupportDetails querySwapChainSupport(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface) {
    vkc::SwapChainSupportDetails details;
    details.capabilities = device.getSurfaceCapabilitiesKHR(surface);

    uint32_t formatsCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatsCount, nullptr);

    if (formatsCount != 0) {
        details.formats = device.getSurfaceFormatsKHR(surface);
    }

    std::vector<vk::PresentModeKHR> presentModes = device.getSurfacePresentModesKHR(surface);

    if (!presentModes.empty()) {
        details.presentModes = presentModes;
    }

    return details;
}

vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eR8G8B8A8Srgb
                && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            return availablePresentMode;
        }
    }

    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D chooseSwapExtend(const uint32_t& width, const uint32_t& height, const vk::SurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {

        vk::Extent2D actualExtent = { width, height };

        actualExtent.width = std::max(capabilities.minImageExtent.width,
                                      std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height,
                                       std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

namespace vkc {

    SwapChain::SwapChain() = default;

    SwapChain::~SwapChain() = default;

    void SwapChain::connect(const std::shared_ptr<vkc::Device>& device, vk::SurfaceKHR surface) {
        m_physicalDevice = device->m_physicalDevice,
        m_device = device->m_logicalDevice;

        if (m_physicalDevice.getSurfaceSupportKHR(device->m_queueFamilyIndices.graphics, surface)) {
            m_surface = surface;
        } else {
            throw std::runtime_error("Device not support Surface");
        }
    }

    void SwapChain::create(uint32_t& width, uint32_t& height, uint32_t graphicsFamilyIndex, uint32_t presetFamilyIndex) {
        vkc::SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_physicalDevice, m_surface);

        vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        vk::Extent2D extent = chooseSwapExtend(width, height, swapChainSupport.capabilities);

        m_imageCount = swapChainSupport.capabilities.minImageCount + 1;

        if (swapChainSupport.capabilities.maxImageCount > 0 &&
                m_imageCount > swapChainSupport.capabilities.maxImageCount) {
            m_imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        vk::SwapchainCreateInfoKHR createInfo{
            .surface = m_surface,
            .minImageCount = m_imageCount,
            .imageFormat = surfaceFormat.format,
            .imageColorSpace = surfaceFormat.colorSpace,
            .imageExtent = extent,
            .imageArrayLayers = 1,
            .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
            .preTransform = swapChainSupport.capabilities.currentTransform,
            .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode = presentMode,
            .clipped = VK_TRUE,
            .oldSwapchain = nullptr
        };

        std::vector<uint32_t> queueFamilyIndices = {
                graphicsFamilyIndex, presetFamilyIndex
        };

        // TODO: Check definition of possibly multiples Family Index in SwapChain
        if (graphicsFamilyIndex != presetFamilyIndex) {
            createInfo.imageSharingMode = vk::SharingMode::eConcurrent,
            createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
            createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
        } else {
            createInfo.imageSharingMode = vk::SharingMode::eExclusive,
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        m_swapChain = m_device.createSwapchainKHR(createInfo);
        m_images = m_device.getSwapchainImagesKHR(m_swapChain);

        m_extent = extent;
        m_format = surfaceFormat.format;
        m_colorSpace = surfaceFormat.colorSpace;

        m_buffers.resize(m_images.size());

        for (size_t i = 0; i < m_imageCount; ++i) {
            vk::ImageViewCreateInfo colorAttachmentView{
                .viewType = vk::ImageViewType::e2D,
                .format = m_format,
                .components = {
                        vk::ComponentSwizzle::eR,
                        vk::ComponentSwizzle::eG,
                        vk::ComponentSwizzle::eB,
                        vk::ComponentSwizzle::eA
                },
                .subresourceRange = {
                        .aspectMask = vk::ImageAspectFlagBits::eColor,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1
                }
            };

            m_buffers[i].image = m_images[i];
            colorAttachmentView.image = m_buffers[i].image;

            m_buffers[i].view = m_device.createImageView(colorAttachmentView);
        }
    }

    vk::Result SwapChain::acquireNextImage(vk::Semaphore presentCompleteSemaphore, uint32_t *imageIndex) {
        return m_device.acquireNextImageKHR(m_swapChain, std::numeric_limits<uint64_t>::max(), presentCompleteSemaphore,
                                            nullptr, imageIndex);
    }

    vk::Result SwapChain::queuePresent(vk::Queue queue, uint32_t imageIndex, vk::Semaphore waitSemaphore) {
        vk::PresentInfoKHR presentInfo{
            .swapchainCount = 1,
            .pSwapchains = &m_swapChain,
            .pImageIndices = &imageIndex
        };

        // Check if a wait semaphore has been specified to wait for before presenting the image
        if (waitSemaphore) {
            presentInfo.pWaitSemaphores = &waitSemaphore;
            presentInfo.waitSemaphoreCount = 1;
        }

        return queue.presentKHR(presentInfo);
    }

    void SwapChain::cleanup() {
        if (m_swapChain) {
            for (uint32_t i = 0; i < m_imageCount; ++i) {
                vkDestroyImageView(m_device, m_buffers[i].view, nullptr);
            }
        }

        m_device.destroySwapchainKHR(m_swapChain);
        m_swapChain = nullptr;
    }

    vk::SwapchainKHR SwapChain::getSwapChain() const {
        return m_swapChain;
    }

    vk::Format SwapChain::getFormat() const {
        return m_format;
    }

    vk::Extent2D SwapChain::getExtent() const {
        return m_extent;
    }

    uint32_t SwapChain::getImageCount() const {
        return m_imageCount;
    }

    vk::ImageView SwapChain::getImageView(size_t index) const {
        return m_buffers[index].view;
    }

    vk::SurfaceKHR SwapChain::getSurface() {
        return m_surface;
    }

} // End namespace vk
