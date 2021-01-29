#ifndef PROTOTYPE_ACTION_RPG_RENDERER_HPP
#define PROTOTYPE_ACTION_RPG_RENDERER_HPP


#include "Instance.hpp"
#include "Device.hpp"
#include "SwapChain.hpp"
#include "Buffer.hpp"
#include "../window/Window.hpp"

namespace core {

    class Renderer {
    public:
        explicit Renderer(std::unique_ptr<Window>& window);

        ~Renderer();

        void init();

        void clean();

        void draw();

        void createRenderPass();

        void createGraphicsPipeline();

        void createFramebuffers();

        void createCommandPool();

        void createSyncObjects();

        void recordCommands();

        void recreateSwapchain();

        void cleanSwapChain();

        void createVertexBuffer();

        void createIndexBuffer();

    private:
        std::unique_ptr<Window>& m_window;

        vk::Instance m_instance;

        vk::Device* m_device{};
        VkPhysicalDevice m_physicalDevice{};
        VkDevice m_logicalDevice{};

        VkQueue m_presentQueue{};
        VkQueue m_graphicsQueue{};

        VkSurfaceKHR m_surface{};

        core::WindowSize m_windowSize;

        vk::SwapChain m_swapChain{};
        std::vector<VkFramebuffer> m_framebuffers;

        VkRenderPass m_renderPass{};

        VkPipelineLayout m_pipelineLayout{};
        VkPipeline m_graphicsPipeline{};

        VkCommandPool m_commandPool{};
        std::vector<VkCommandBuffer> m_commandBuffers;

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
