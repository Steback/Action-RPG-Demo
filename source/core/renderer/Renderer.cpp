#include "Renderer.hpp"

#include "Constants.hpp"


namespace core {

    Renderer::Renderer(std::unique_ptr<Window>& window) {
        VkApplicationInfo appInfo{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Prototype Action RPG",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "Custom Engine",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_API_VERSION_1_2
        };

        mInstance.init(appInfo);
        mInstance.createSurface(window->mWindow, mSurface);
        mInstance.pickPhysicalDevice(mPhysicalDevice, mSurface);
        mDevice.init(mPhysicalDevice, mSurface, mGraphicsQueue, mPresentQueue);
        mDevice.createSwapChain(mSwapChain, window->mWindow, mSurface);
        mDevice.createImageViews(mSwapChain);
        mDevice.createRenderPass(mRenderPass, mSwapChain.mImageFormat);
        mDevice.createGraphicsPipeline(mGraphicsPipeline, mPipelineLayout, mSwapChain.mExtent, mRenderPass);
        mDevice.createFramebuffers(mSwapChain, mRenderPass);
        mDevice.createCommandPool(mCommandPool, mPhysicalDevice.device, mSurface);
        mDevice.createCommandBuffers(mCommandBuffers, mCommandPool, mSwapChain.mFramebuffers);
        recordCommands({0.0f, 0.0f, 0.0f, 1.0f});

        mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        mRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        mFences.resize(MAX_FRAMES_IN_FLIGHT);
        mImageFences.resize(mSwapChain.mImageViews.size(), VK_NULL_HANDLE);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            mDevice.createSemaphore(mImageAvailableSemaphores[i]);
            mDevice.createSemaphore(mRenderFinishedSemaphores[i]);
            mDevice.createFence(mFences[i]);
        }

        spdlog::info("[Renderer] Initialized");
    }

    Renderer::~Renderer() = default;

    void Renderer::draw() {
        mDevice.waitForFence(mFences[currentFrame]);

        uint32_t indexImage;

        mDevice.acquireNextImage(indexImage, mSwapChain.mSwapChain, mImageAvailableSemaphores[currentFrame]);

        if (mImageFences[indexImage] != VK_NULL_HANDLE) {
            mDevice.waitForFence(mImageFences[indexImage]);
        }

        mImageFences[indexImage] = mFences[currentFrame];

        std::array<VkSemaphore, 1> waitSemaphores = { mImageAvailableSemaphores[currentFrame] };
        std::array<VkPipelineStageFlags, 1> waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        std::array<VkSemaphore, 1> signalSemaphores = { mRenderFinishedSemaphores[currentFrame] };

        VkSubmitInfo submitInfo{
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
                .pWaitSemaphores = waitSemaphores.data(),
                .pWaitDstStageMask = waitStages.data(),
                .commandBufferCount = 1,
                .pCommandBuffers = &mCommandBuffers[indexImage],
                .signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
                .pSignalSemaphores = signalSemaphores.data()
        };

        mDevice.resetFence(mFences[currentFrame]);

        vk::resultValidation(vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mFences[currentFrame]),
                             "Failed to submit draw command buffer");

        std::array<VkSwapchainKHR, 1> swapChains = { mSwapChain.mSwapChain };

        VkPresentInfoKHR presentInfo{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
            .pWaitSemaphores = signalSemaphores.data(),
            .swapchainCount = static_cast<uint32_t>(swapChains.size()),
            .pSwapchains = swapChains.data(),
            .pImageIndices = &indexImage,
            .pResults = nullptr
        };

        vkQueuePresentKHR(mPresentQueue, &presentInfo);
        vkQueueWaitIdle(mPresentQueue);

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void Renderer::clean() {
        mDevice.waitIdle();

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            mDevice.destroySemaphore(mImageAvailableSemaphores[i]);
            mDevice.destroySemaphore(mRenderFinishedSemaphores[i]);
            mDevice.destroyFence(mFences[i]);
        }

        mDevice.destroyCommandPool(mCommandPool);
        mDevice.destroyFramebuffers(mSwapChain.mFramebuffers);
        mDevice.destroyGraphicsPipeline(mGraphicsPipeline, mPipelineLayout);
        mDevice.destroyRenderPass(mRenderPass);
        mDevice.destroyImageViews(mSwapChain);
        mDevice.destroySwapChain(mSwapChain);
        mDevice.destroy();
        mInstance.destroySurface(mSurface);
        mInstance.destroy();

        spdlog::info("[Renderer] Cleaned");
    }

    void Renderer::recordCommands(const glm::vec4& clearColor) {
        std::array<VkClearValue, 1> clearColors = {
                { clearColor.x, clearColor.y, clearColor.z, clearColor.w }
        };

        for (size_t i = 0; i < mCommandBuffers.size(); ++i) {
            VkCommandBufferBeginInfo beginInfo{
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                    .pInheritanceInfo = nullptr
            };

            vk::resultValidation(vkBeginCommandBuffer(mCommandBuffers[i], &beginInfo),
                             "Failed to begin recording command buffer");

            VkRenderPassBeginInfo renderPassInfo{
                    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                    .renderPass = mRenderPass,
                    .framebuffer = mSwapChain.mFramebuffers[i],
                    .renderArea = {
                            .offset = { 0, 0 },
                            .extent = mSwapChain.mExtent
                    },
                    .clearValueCount = static_cast<uint32_t>(clearColors.size()),
                    .pClearValues = clearColors.data()
            };

                vkCmdBeginRenderPass(mCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

                    vkCmdBindPipeline(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);
                    vkCmdDraw(mCommandBuffers[i], 3, 1, 0, 0);

                vkCmdEndRenderPass(mCommandBuffers[i]);

            vk::resultValidation(vkEndCommandBuffer(mCommandBuffers[i]),
                                 "Failed to record command buffer");
        }
    }

} // End namespace core