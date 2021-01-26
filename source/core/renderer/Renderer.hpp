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

        void recordCommands();

        void recreateSwapchain();

        void cleanSwapChain();

        void createVertexBuffer();

        void createIndexBuffer();

    private:
        std::unique_ptr<Window>& m_window;

        vk::Instance m_instance;
        vk::PhysicalDevice m_physicialDevice;
        vk::Device m_device;

        VkQueue m_presentQueue{};
        VkQueue m_graphicsQueue{};

        VkSurfaceKHR m_surface{};

        vk::SwapChain m_swapchain{};

        VkRenderPass m_renderPass{};

        VkPipelineLayout m_pipelineLayout{};
        VkPipeline m_graphicsPipeline{};

        vk::CommandPool m_commandPool;
        VkCommandPool m_transferCommandPool{};

        std::vector<VkSemaphore> m_imageAvailableSemaphores{};
        std::vector<VkSemaphore> m_renderFinishedSemaphores{};
        std::vector<VkFence> m_fences;
        std::vector<VkFence> m_imageFences;

        size_t m_currentFrame = 0;

        vk::Buffer m_vertexBuffer;
        vk::Buffer m_indexBuffer;
    };

} // End namespace core


#endif //PROTOTYPE_ACTION_RPG_RENDERER_HPP
