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
        mDevice.createFramebuffers(mSwapChainFramebuffers, mSwapChain, mRenderPass);

        spdlog::info("[Renderer] Initialized");
    }

    Renderer::~Renderer() = default;

    void Renderer::draw() {

    }

    void Renderer::clean() {
        mDevice.destroyFramebuffers(mSwapChainFramebuffers);
        mDevice.destroyGraphicsPipeline(mGraphicsPipeline, mPipelineLayout);
        mDevice.destroyRenderPass(mRenderPass);
        mDevice.destroyImageViews(mSwapChain);
        mDevice.destroySwapChain(mSwapChain);
        mDevice.destroy();
        mInstance.destroySurface(mSurface);
        mInstance.destroy();

        spdlog::info("[Renderer] Cleaned");
    }

} // End namespace core