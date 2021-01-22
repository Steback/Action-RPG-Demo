#ifndef PROTOTYPE_ACTION_RPG_DEVICE_HPP
#define PROTOTYPE_ACTION_RPG_DEVICE_HPP


#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

#include "PhysicalDevice.hpp"
#include "SwapChain.hpp"


namespace vk {

    class Device {
    public:
        Device();

        ~Device();

        void init(const PhysicalDevice& physicalDevice, VkSurfaceKHR& surface, VkQueue& graphicsQueue,
                  VkQueue& presentQueue);

        void destroy();

        void waitIdle();

        void createSwapChain(SwapChain& swapChain, GLFWwindow* window, VkSurfaceKHR surface);

        void destroySwapChain(SwapChain& swapChain);

        void createImageViews(SwapChain& swapChain);

        void destroyImageViews(SwapChain& swapChain);

        void createGraphicsPipeline(VkPipeline& graphicsPipeline, VkPipelineLayout& pipelineLayout,
                                    const VkExtent2D& swapChainExtend, const VkRenderPass& renderPass);

        void destroyGraphicsPipeline(VkPipeline& graphicsPipeline, VkPipelineLayout& pipelineLayout);

        VkShaderModule createShaderModule(const std::vector<char>& code);

        void destroyShaderModule(VkShaderModule& shader);

        void createRenderPass(VkRenderPass& renderPass, const VkFormat& swapChainFormat);

        void destroyRenderPass(VkRenderPass& renderPass);

        void createFramebuffers(SwapChain& swapChain, const VkRenderPass& renderPass);

        void destroyFramebuffers(std::vector<VkFramebuffer>& swapChainFramebuffers);

        void createCommandPool(VkCommandPool& commandPool, const VkPhysicalDevice& physicalDevice,
                               const VkSurfaceKHR& surface);

        void destroyCommandPool(VkCommandPool& commandPool);

        void createCommandBuffers(std::vector<VkCommandBuffer>& commandBuffers, const VkCommandPool& commandPool,
                                  const std::vector<VkFramebuffer>& swapChainFramebuffers);

        void createSemaphore(VkSemaphore& semaphore);

        void destroySemaphore(VkSemaphore& semaphore);

        void acquireNextImage(uint32_t& imageIndex, const VkSwapchainKHR& swapchain,
                              const VkSemaphore& imageAvailableSemaphore);

        void createFence(VkFence& fence);

        void destroyFence(VkFence& fence);

        void waitForFence(const VkFence& fence);

        void resetFence(const VkFence& fence);

    private:
        VkDevice mDevice{};
        PhysicalDevice mPhysicalDevice;
    };

} // End namespace vk


#endif //PROTOTYPE_ACTION_RPG_DEVICE_HPP
