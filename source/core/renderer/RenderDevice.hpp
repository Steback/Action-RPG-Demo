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
#include "../resources/ModelInterface.hpp"
#include "../resources/ResourceManager.hpp"


namespace core {

    class CommandList;

    class RenderDevice {
    public:
        explicit RenderDevice(std::shared_ptr<Window> window, vk::Instance instance, const std::string& appName, std::shared_ptr<core::Device> device, vk::SurfaceKHR surface);

        ~RenderDevice();

        void init();

        void cleanup(const std::shared_ptr<core::Instance>& instance);

        void acquireNextImage();

        void render();

        void createRenderPass();

        void createGraphicsPipeline();

        void createFramebuffers();

        void createSyncObjects();

        void recreateSwapchain();

        void cleanSwapChain();

        void createDescriptorSetLayout();

        void createDescriptorPool();

        void createDescriptorSets();

        void createDepthResources();

        void createMsaaResources();

        void updateVP(const glm::mat4& view, const glm::mat4& proj);

        vk::Queue& getGraphicsQueue();

        std::shared_ptr<CommandList> addCommandList();

        vk::Framebuffer& getFrameBuffer();

        vk::Extent2D getSwapChainExtent();

        vk::RenderPass& getRenderPass();

        vk::DescriptorSet& getDescriptorSet();

        std::shared_ptr<GraphicsPipeline> addPipeline(std::shared_ptr<core::Shader> shaderID, vk::Device device, bool inited = false);

    public:
        MVP m_mvp{};

    private:
        std::shared_ptr<Window> m_window;

        std::shared_ptr<core::Device> m_device{};
        vk::PhysicalDevice m_physicalDevice{};
        vk::Device m_logicalDevice{};

        vk::Queue m_graphicsQueue{};

        core::WindowSize m_windowSize;

        core::SwapChain m_swapChain{};
        std::vector<vk::Framebuffer> m_framebuffers;

        vk::RenderPass m_renderPass{};

        std::vector<std::shared_ptr<CommandList>> m_commands;

        std::vector<vk::Semaphore> m_imageAvailableSemaphores{};
        std::vector<vk::Semaphore> m_renderFinishedSemaphores{};
        std::vector<vk::Fence> m_fences;
        std::vector<vk::Fence> m_imageFences;

        size_t m_currentFrame = 0;
        uint32_t m_indexImage{};

        vk::DescriptorSetLayout m_descriptorSetLayout{};
        vk::DescriptorPool m_descriptorPool{};
        std::vector<vk::DescriptorSet> m_descriptorSets;

        // Pipelines
        std::vector<std::shared_ptr<GraphicsPipeline>> m_pipelines;

        // ImGui
        core::UIImGui m_ui;

        // Textures
        vk::SampleCountFlagBits m_msaaSamples = vk::SampleCountFlagBits::e1;

        // Multisampling anti-aliasing
        core::Image m_colorImage;

        // TODO: Check for optimising in depth buffer
        core::Image m_depthBuffer;
        vk::Format m_depthFormat{};
    };

} // End namespace core


#endif //PROTOTYPE_ACTION_RPG_RENDERDEVICE_HPP
