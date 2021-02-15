#ifndef PROTOTYPE_ACTION_RPG_RENDERER_HPP
#define PROTOTYPE_ACTION_RPG_RENDERER_HPP


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
#include "../components/Model.hpp"
#include "../texture/ResourceManager.hpp"


namespace core {

    class Renderer {
    public:
        explicit Renderer(std::unique_ptr<Window>& window, VkInstance instance, const std::string& appName, vk::Device* device, VkSurfaceKHR surface);

        ~Renderer();

        void init(core::ResourceManager* resourceManager);

        void cleanup();

        void draw(entt::registry& registry);

        void drawFrame(entt::registry& registry);

        void createRenderPass();

        void createGraphicsPipeline();

        void createFramebuffers();

        void createCommandPool();

        void createSyncObjects();

        void recordCommands(uint32_t indexImage, entt::registry& registry);

        void recreateSwapchain();

        void cleanSwapChain();

        void createDescriptorSetLayout();

        void createDescriptorPool();

        void createDescriptorSets();

        void createDepthResources();

        void createMsaaResources();

        void createPushConstants();

        void updateVP(const glm::mat4& view, const glm::mat4& proj);

        VkPhysicalDevice& getPhysicalDevice();

        VkQueue& getGraphicsQueue();

    private:
        std::unique_ptr<Window>& m_window;

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

        VkPushConstantRange m_mvpRange{};

        VkDescriptorSetLayout m_descriptorSetLayout{};
        VkDescriptorPool m_descriptorPool{};
        std::vector<VkDescriptorSet> m_descriptorSets;

        // ImGui
        core::UIImGui m_ui;

        // Textures
        core::ResourceManager* m_resourceManager{};
        VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;

        // Multisampling anti-aliasing
        vk::Image m_colorImage;

        // TODO: Check for optimising in depth buffer
        vk::Image m_depthBuffer;
        VkFormat m_depthFormat{};

        MVP m_mvp{};
    };

} // End namespace core


#endif //PROTOTYPE_ACTION_RPG_RENDERER_HPP
