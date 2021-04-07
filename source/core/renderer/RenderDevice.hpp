#ifndef PROTOTYPE_ACTION_RPG_RENDERDEVICE_HPP
#define PROTOTYPE_ACTION_RPG_RENDERDEVICE_HPP


#include <memory>
#include <vector>

#include "entt/entt.hpp"

#include "Instance.hpp"
#include "Device.hpp"
#include "SwapChain.hpp"
#include "Buffer.hpp"
#include "Image.hpp"
#include "UIImGui.hpp"
#include "GraphicsPipeline.hpp"
#include "../Utilities.hpp"
#include "../camera/Camera.hpp"
#include "../window/Window.hpp"
#include "../resources/Model.hpp"
#include "../resources/ResourceManager.hpp"


namespace core {

    class RenderDevice {
    public:
        explicit RenderDevice(std::shared_ptr<Window> window, vk::Instance instance, const std::string& appName, std::shared_ptr<vkc::Device> device, vk::SurfaceKHR surface);

        ~RenderDevice();

        void init(bool drawGrid = false);

        void cleanup(const std::shared_ptr<vkc::Instance>& instance);

        void render(const glm::vec4& clearColor);

        void createRenderPass();

        void createGraphicsPipeline();

        void createFramebuffers();

        void createCommandPool();

        void createSyncObjects();

        void recreateSwapchain();

        void cleanSwapChain();

        void createDescriptorSetLayout();

        void createDescriptorPool();

        void createDescriptorSets();

        void createDepthResources();

        void createMsaaResources();

        void createPushConstants();

        void createGridPipeline();

        void updateVP(const glm::mat4& view, const glm::mat4& proj);

        vk::Queue& getGraphicsQueue();

        void renderMesh(const core::Mesh& mesh, const glm::mat4& matrix);

        void beginRenderPass(const glm::vec4& clearColor);

        void endRenderPass();

        void drawGrid();

    private:
        std::shared_ptr<Window> m_window;

        std::shared_ptr<vkc::Device> m_device{};
        vk::PhysicalDevice m_physicalDevice{};
        vk::Device m_logicalDevice{};

        vk::Queue m_graphicsQueue{};

        core::WindowSize m_windowSize;

        vkc::SwapChain m_swapChain{};
        std::vector<vk::Framebuffer> m_framebuffers;

        vk::RenderPass m_renderPass{};

        std::unique_ptr<vkc::GraphicsPipeline> m_pipeline;

        vk::CommandPool m_commandPool{};
        std::vector<vk::CommandBuffer> m_commandBuffers;

        std::vector<vk::Semaphore> m_imageAvailableSemaphores{};
        std::vector<vk::Semaphore> m_renderFinishedSemaphores{};
        std::vector<vk::Fence> m_fences;
        std::vector<vk::Fence> m_imageFences;

        size_t m_currentFrame = 0;
        uint32_t m_indexImage{};

        vk::PushConstantRange m_mvpRange{};

        vk::DescriptorSetLayout m_descriptorSetLayout{};
        vk::DescriptorPool m_descriptorPool{};
        std::vector<vk::DescriptorSet> m_descriptorSets;

        // ImGui
        core::UIImGui m_ui;

        // Textures
        vk::SampleCountFlagBits m_msaaSamples = vk::SampleCountFlagBits::e1;

        // Multisampling anti-aliasing
        vkc::Image m_colorImage;

        // TODO: Check for optimising in depth buffer
        vkc::Image m_depthBuffer;
        vk::Format m_depthFormat{};

        MVP m_mvp{};

        // Editor grid
        bool m_drawGrid{};
        std::unique_ptr<vkc::GraphicsPipeline> m_gridPipeline;
    };

} // End namespace core


#endif //PROTOTYPE_ACTION_RPG_RENDERDEVICE_HPP
