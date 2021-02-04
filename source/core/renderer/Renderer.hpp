#ifndef PROTOTYPE_ACTION_RPG_RENDERER_HPP
#define PROTOTYPE_ACTION_RPG_RENDERER_HPP


#include "Instance.hpp"
#include "Device.hpp"
#include "SwapChain.hpp"
#include "Buffer.hpp"
#include "Image.hpp"
#include "../Utilities.hpp"
#include "../camera/Camera.hpp"
#include "../window/Window.hpp"
#include "../model/Model.hpp"
#include "../texture/Texture.hpp"


namespace core {

    class Renderer {
    public:
        explicit Renderer(std::unique_ptr<Window>& window);

        ~Renderer();

        void init();

        void initUI();

        void cleanup();

        void draw();

        void drawFrame();

        void drawUI();

        void createRenderPass();

        void createUIRenderPass();

        void createGraphicsPipeline();

        void createFramebuffers();

        void createUIFramebuffers();

        void createCommandPool();

        void createUICommandPool();

        void createSyncObjects();

        void recordCommands(uint32_t bufferIdx);

        void recordUICommands(uint32_t bufferIdx);

        void recreateSwapchain();

        void cleanSwapChain();

        void createDescriptorSetLayout();

        void createUniformBuffers();

        void updateUniformBuffer(uint32_t currentImage);

        void createDescriptorPool();

        void createDescriptorSets();

        void createUIDescriptorPool();

        uint createTexture(const std::string& fileName);

        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

        void createTextureSampler();

        void createDepthResources();

        void createMeshModel(const std::string &modelFile);

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

        // TODO: Create UI class
        VkRenderPass m_uiRenderPass{};
        std::vector<VkFramebuffer> m_uiFramebuffers{};
        VkCommandPool m_uiCommandPool{};
        std::vector<VkCommandBuffer> m_uiCommandBuffers{};
        VkDescriptorPool m_uiDescriptorPool{};

        // TODO: Create Texture Manager Class with is oen descriptor pool and layout
        std::vector<core::Texture> textures;
        VkDescriptorPool samplerDescriptorPool{};
        VkDescriptorSetLayout samplerSetLayout{};
        VkSampler m_textureSampler{};

        // TODO: Check for optimising in depth buffer
        vk::Image m_depthBuffer;
        VkFormat m_depthFormat;

        // TODO: Temp object
        core::Model vikingRoom{};

        UniformBufferObject ubo{};
        core::Camera camera;
        glm::vec3 m_position{};
        glm::vec3 m_size{};
        float m_angle{};
    };

} // End namespace core


#endif //PROTOTYPE_ACTION_RPG_RENDERER_HPP
