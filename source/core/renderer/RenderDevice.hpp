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
#include "../Utilities.hpp"
#include "../camera/Camera.hpp"
#include "../window/Window.hpp"
#include "../resources/Model.hpp"
#include "../resources/ResourceManager.hpp"


namespace core {

    class RenderDevice {
    public:
        explicit RenderDevice(std::shared_ptr<Window> window, VkInstance instance, const std::string& appName, std::shared_ptr<vk::Device> device, VkSurfaceKHR surface);

        ~RenderDevice();

        void init(bool drawGrid = false);

        void cleanup();

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

        void createGridPipeline(VkGraphicsPipelineCreateInfo& createInfo);

        void updateVP(const glm::mat4& view, const glm::mat4& proj);

        VkQueue& getGraphicsQueue();

        void renderMesh(const core::Mesh& mesh, const glm::mat4& matrix);

        void beginRenderPass(const glm::vec4& clearColor);

        void endRenderPass();

        void setPipeline();

        void drawGrid();

    private:
        std::shared_ptr<Window> m_window;

        std::shared_ptr<vk::Device> m_device{};
        VkPhysicalDevice m_physicalDevice{};
        VkDevice m_logicalDevice{};

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
        uint32_t m_indexImage{};

        VkPushConstantRange m_mvpRange{};

        VkDescriptorSetLayout m_descriptorSetLayout{};
        VkDescriptorPool m_descriptorPool{};
        std::vector<VkDescriptorSet> m_descriptorSets;

        // ImGui
        core::UIImGui m_ui;

        // Textures
        VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;

        // Multisampling anti-aliasing
        vk::Image m_colorImage;

        // TODO: Check for optimising in depth buffer
        vk::Image m_depthBuffer;
        VkFormat m_depthFormat{};

        MVP m_mvp{};

        // Editor grid
        bool m_drawGrid{};
        VkPipeline m_gridPipeline{};
        VkPipelineLayout m_gridPipelineLayout{};
    };

} // End namespace core


#endif //PROTOTYPE_ACTION_RPG_RENDERDEVICE_HPP
