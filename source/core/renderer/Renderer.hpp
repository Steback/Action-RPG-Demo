#ifndef PROTOTYPE_ACTION_RPG_RENDERER_HPP
#define PROTOTYPE_ACTION_RPG_RENDERER_HPP


#include "Instance.hpp"
#include "PhysicalDevice.hpp"
#include "Device.hpp"
#include "../window/Window.hpp"
#include "SwapChain.hpp"


namespace core {

    class Renderer {
    public:
        explicit Renderer(std::unique_ptr<Window>& window);

        ~Renderer();

        void draw();

        void clean();

    private:
        vk::Instance mInstance;
        vk::PhysicalDevice mPhysicalDevice;
        vk::Device mDevice;
        VkSurfaceKHR mSurface{};
        vk::SwapChain mSwapChain{};
        VkPipelineLayout mPipelineLayout;
    };

} // End namespace core


#endif //PROTOTYPE_ACTION_RPG_RENDERER_HPP
