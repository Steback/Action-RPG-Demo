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
    }

    Renderer::~Renderer() = default;

    void Renderer::draw() {

    }

    void Renderer::clean() {
        mDevice.destroySwapChain(mSwapChain);
        mDevice.destroy();
        mInstance.destroySurface(mSurface);
        mInstance.destroy();
    }

} // End namespace core