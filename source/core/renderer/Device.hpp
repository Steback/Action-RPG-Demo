#ifndef PROTOTYPE_ACTION_RPG_DEVICE_HPP
#define PROTOTYPE_ACTION_RPG_DEVICE_HPP


#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

#include "PhysicalDevice.hpp"
#include "SwapChain.hpp"
#include "Buffer.hpp"
#include "../mesh/Mesh.hpp"
#include "../window/Window.hpp"


namespace vk {

    // TODO: Check optimized file struct the Command Pools and his buffers
    struct CommandPool {
        VkCommandPool mPool{};
        std::vector<VkCommandBuffer> mBuffers;
    };

    class Device {
    public:
        Device();

        ~Device();

        void init(const PhysicalDevice& physicalDevice, VkSurfaceKHR& surface, VkQueue& graphicsQueue,
                  VkQueue& presentQueue);

        void destroy();

        void waitIdle();

        void createSwapChain(SwapChain& swapChain, const core::WindowSize& windowSize, VkSurfaceKHR surface, bool recreate = false);

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

        void createCommandPool(VkCommandPool& commandPool, const VkSurfaceKHR& surface);

        void destroyCommandPool(VkCommandPool& commandPool);

        void createCommandBuffers(CommandPool& commandPool, const std::vector<VkFramebuffer>& swapChainFramebuffers);

        void freeCommandBuffers(CommandPool& commandPool);

        void createSemaphore(VkSemaphore& semaphore);

        void destroySemaphore(VkSemaphore& semaphore);

        VkResult acquireNextImage(uint32_t& imageIndex, const VkSwapchainKHR& swapchain,
                              const VkSemaphore& imageAvailableSemaphore);

        void createFence(VkFence& fence);

        void destroyFence(VkFence& fence);

        void waitForFence(const VkFence& fence);

        void resetFence(const VkFence& fence);

        void createBuffer(Buffer& buffer, const VkDeviceSize& size, const VkBufferUsageFlags& flags,
                          const VkMemoryPropertyFlags& properties, const VkSharingMode& sharingMode = VK_SHARING_MODE_EXCLUSIVE);

        void destroyBuffer(Buffer& buffer);

        // TODO: Optimize data copy to memory copy
        template<typename T>
        void mapMemory(const Buffer& buffer, const VkDeviceSize& size, const T* pData) {
            void* data;

            vkMapMemory(mDevice, buffer.mDeviceMemory, 0, size, 0, &data);
                memcpy(data, pData, static_cast<size_t>(size));
            vkUnmapMemory(mDevice, buffer.mDeviceMemory);
        }

    private:
        VkDevice mDevice{};
        PhysicalDevice mPhysicalDevice;
    };

} // End namespace vk


#endif //PROTOTYPE_ACTION_RPG_DEVICE_HPP
