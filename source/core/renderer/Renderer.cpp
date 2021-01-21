#include "Renderer.hpp"


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
        mDevice.init(mPhysicalDevice, mSurface);
        mDevice.createSwapChain(mSwapChain, window->mWindow, mSurface);
        mDevice.createImageViews(mSwapChain);
        mDevice.createRenderPass(mRenderPass, mSwapChain.mImageFormat);
        mDevice.createGraphicsPipeline(mGraphicsPipeline, mPipelineLayout, mSwapChain.mExtent, mRenderPass);
        mDevice.createFramebuffers(mSwapChain, mRenderPass);
        mDevice.createCommandPool(mCommandPool, mPhysicalDevice.device, mSurface);
        mDevice.createCommandBuffers(mCommandBuffers, mCommandPool, mSwapChain.mFramebuffers);
        recordCommands({0.0f, 0.0f, 0.0f, 1.0f});

        spdlog::info("[Renderer] Initialized");
    }

    Renderer::~Renderer() = default;

    void Renderer::draw() {

    }

    void Renderer::clean() {
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
                    .flags = 0,
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