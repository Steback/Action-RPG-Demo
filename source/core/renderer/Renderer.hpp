#ifndef PROTOTYPE_ACTION_RPG_RENDERER_HPP
#define PROTOTYPE_ACTION_RPG_RENDERER_HPP


#include "Instance.hpp"
#include "PhysicalDevice.hpp"
#include "Device.hpp"
#include "SwapChain.hpp"
#include "Buffer.hpp"
#include "../window/Window.hpp"


namespace core {

    class Renderer {
    public:
        explicit Renderer(std::unique_ptr<Window>& window);

        ~Renderer();

        void draw();

        void clean();

        void recordCommands(const glm::vec4& clearColor);

        void recreateSwapchain();

        void cleanSwapChain();

    private:
        std::unique_ptr<Window>& mWindow;
        vk::Instance mInstance;
        vk::PhysicalDevice mPhysicalDevice;
        vk::Device mDevice;
        VkQueue mPresentQueue{};
        VkQueue mGraphicsQueue{};
        VkSurfaceKHR mSurface{};
        vk::SwapChain mSwapChain{};
        VkPipelineLayout mPipelineLayout{};
        VkRenderPass mRenderPass{};
        VkPipeline mGraphicsPipeline{};
        vk::CommandPool mCommandPool;
        std::vector<VkSemaphore> mImageAvailableSemaphores{};
        std::vector<VkSemaphore> mRenderFinishedSemaphores{};
        std::vector<VkFence> mFences;
        std::vector<VkFence> mImageFences;
        size_t currentFrame = 0;
        vk::Buffer mVertexBuffer;
    };

} // End namespace core


#endif //PROTOTYPE_ACTION_RPG_RENDERER_HPP
