#include "Renderer.hpp"

#include "Constants.hpp"
#include "../mesh/Mesh.hpp"


namespace core {

    const std::vector<Vertex> vertices = {
            { {0.0f, -0.5f}, {1.0f, 0.0f, 0.0f} },
            { {0.5f, 0.5f}, {0.0f, 1.0f, 0.0f} },
            { {-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f} }
    };

    Renderer::Renderer(std::unique_ptr<Window>& window) : mWindow(window) {
        VkApplicationInfo appInfo{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Prototype Action RPG",
            .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
            .pEngineName = "Custom Engine",
            .engineVersion = VK_MAKE_VERSION(0, 1, 0),
            .apiVersion = VK_API_VERSION_1_2
        };

        mInstance.init(appInfo);
        mInstance.createSurface(mWindow->mWindow, mSurface);
        mInstance.pickPhysicalDevice(mPhysicalDevice, mSurface);
        mDevice.init(mPhysicalDevice, mSurface, mGraphicsQueue, mPresentQueue);
        mDevice.createSwapChain(mSwapChain, mWindow->getSize(), mSurface);
        mDevice.createImageViews(mSwapChain);
        mDevice.createRenderPass(mRenderPass, mSwapChain.mImageFormat);
        mDevice.createGraphicsPipeline(mGraphicsPipeline, mPipelineLayout, mSwapChain.mExtent, mRenderPass);
        mDevice.createFramebuffers(mSwapChain, mRenderPass);
        mDevice.createCommandPool(mCommandPool.mPool, mSurface);
        mDevice.createBuffer(mVertexBuffer, sizeof(Vertex) * vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        mDevice.mapMemory(mVertexBuffer, sizeof(Vertex) * vertices.size(), vertices.data());
        mDevice.createCommandBuffers(mCommandPool, mSwapChain.mFramebuffers);
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

        VkResult result = mDevice.acquireNextImage(indexImage, mSwapChain.mSwapChain,
                                                   mImageAvailableSemaphores[currentFrame]);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapchain();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            spdlog::throw_spdlog_ex("Failed to acquire swap chain image");
        }

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
                .pCommandBuffers = &mCommandPool.mBuffers[indexImage],
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

        result = vkQueuePresentKHR(mPresentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mWindow->mResize) {
            mWindow->mResize = false;
            recreateSwapchain();
        } else if (result != VK_SUCCESS) {
            spdlog::throw_spdlog_ex("Failed to present swap chain image");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        vkQueueWaitIdle(mPresentQueue);
    }

    void Renderer::clean() {
        mDevice.waitIdle();

        cleanSwapChain();

        mDevice.destroyBuffer(mVertexBuffer);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            mDevice.destroySemaphore(mImageAvailableSemaphores[i]);
            mDevice.destroySemaphore(mRenderFinishedSemaphores[i]);
            mDevice.destroyFence(mFences[i]);
        }

        mDevice.destroyCommandPool(mCommandPool.mPool);
        mDevice.destroy();
        mInstance.destroySurface(mSurface);
        mInstance.destroy();

        spdlog::info("[Renderer] Cleaned");
    }

    void Renderer::recordCommands(const glm::vec4& clearColor) {
        std::array<VkClearValue, 1> clearColors = {
                { clearColor.x, clearColor.y, clearColor.z, clearColor.w }
        };

        for (size_t i = 0; i < mCommandPool.mBuffers.size(); ++i) {
            VkCommandBufferBeginInfo beginInfo{
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                    .pInheritanceInfo = nullptr
            };

            vk::resultValidation(vkBeginCommandBuffer(mCommandPool.mBuffers[i], &beginInfo),
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

                vkCmdBeginRenderPass(mCommandPool.mBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

                    vkCmdBindPipeline(mCommandPool.mBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);

                    std::array<VkBuffer, 1> vertexBuffer = { mVertexBuffer.mBuffer };
                    std::array<VkDeviceSize, 1> offsets = { 0 };
                    vkCmdBindVertexBuffers(mCommandPool.mBuffers[i], 0, 1, vertexBuffer.data(), offsets.data());

                    vkCmdDraw(mCommandPool.mBuffers[i], static_cast<uint32_t>(vertices.size()), 1, 0, 0);

                vkCmdEndRenderPass(mCommandPool.mBuffers[i]);

            vk::resultValidation(vkEndCommandBuffer(mCommandPool.mBuffers[i]),
                                 "Failed to record command buffer");
        }
    }

    void Renderer::recreateSwapchain() {
        // TODO: Optimize window resize
        while (mWindow->mSize.mWidth == 0 || mWindow->mSize.mHeight == 0)  {
            glfwWaitEvents();
        }

        mDevice.waitIdle();

        cleanSwapChain();

        mDevice.createSwapChain(mSwapChain, mWindow->getSize(), mSurface, true);
        mDevice.createImageViews(mSwapChain);
        mDevice.createRenderPass(mRenderPass, mSwapChain.mImageFormat);
        mDevice.createGraphicsPipeline(mGraphicsPipeline, mPipelineLayout, mSwapChain.mExtent, mRenderPass);
        mDevice.createFramebuffers(mSwapChain, mRenderPass);
        mDevice.createCommandBuffers(mCommandPool, mSwapChain.mFramebuffers);
        recordCommands({0.0f, 0.0f, 0.0f, 1.0f});
    }

    void Renderer::cleanSwapChain() {
        mDevice.destroyFramebuffers(mSwapChain.mFramebuffers);
        mDevice.freeCommandBuffers(mCommandPool);
        mDevice.destroyGraphicsPipeline(mGraphicsPipeline, mPipelineLayout);
        mDevice.destroyRenderPass(mRenderPass);
        mDevice.destroyImageViews(mSwapChain);
        mDevice.destroySwapChain(mSwapChain);
    }

} // End namespace core