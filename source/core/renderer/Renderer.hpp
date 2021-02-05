#ifndef PROTOTYPE_ACTION_RPG_RENDERER_HPP
#define PROTOTYPE_ACTION_RPG_RENDERER_HPP


#include <memory>

#include "Instance.hpp"
#include "Device.hpp"
#include "SwapChain.hpp"
#include "Buffer.hpp"
#include "Image.hpp"
#include "UIImGui.hpp"
#include "../Utilities.hpp"
#include "../camera/Camera.hpp"
#include "../window/Window.hpp"
#include "../model/Model.hpp"
#include "../texture/TextureManager.hpp"


namespace core {

    class Renderer {
    public:
        explicit Renderer(std::unique_ptr<Window>& window);

        ~Renderer();

        void init();

        void cleanup();

        void draw();

        void drawFrame();

        void drawUI();

        void createRenderPass();

        void createGraphicsPipeline();

        void createFramebuffers();

        void createCommandPool();

        void createSyncObjects();

        void recordCommands(uint32_t bufferIdx);

        void recreateSwapchain();

        void cleanSwapChain();

        void createDescriptorSetLayout();

        void createUniformBuffers();

        void updateUniformBuffer(uint32_t currentImage);

        void createDescriptorPool();

        void createDescriptorSets();

        void createDepthResources();

        void createModel(const std::string &modelFile);

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

        std::vector<vk::Buffer> m_uniformBuffers;

        VkDescriptorSetLayout m_descriptorSetLayout{};
        VkDescriptorPool m_descriptorPool{};
        std::vector<VkDescriptorSet> m_descriptorSets;

        // ImGui
        core::UIImGui m_ui;

        // Textures
        std::unique_ptr<core::TextureManager> m_texturesManager;

        // TODO: Check for optimising in depth buffer
        vk::Image m_depthBuffer;
        VkFormat m_depthFormat{};

        // TODO: Temp object
        core::Model vikingRoom{};

        UniformBufferObject ubo{};
        core::Camera camera;
        glm::vec3 m_position{};
        glm::vec3 m_size{};
        float m_angle{};
        float deltaTime = 0.0f;
        float lastTime = 0.0f;
        bool increment = true;
    };

} // End namespace core


#endif //PROTOTYPE_ACTION_RPG_RENDERER_HPP
