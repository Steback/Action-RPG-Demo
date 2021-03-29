#ifndef PROTOTYPE_ACTION_RPG_UIIMGUI_HPP
#define PROTOTYPE_ACTION_RPG_UIIMGUI_HPP


#include <vector>
#include <memory>

#include "vulkan/vulkan.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"


#include "SwapChain.hpp"
#include "Device.hpp"


namespace core {

    class UIImGui {
    public:
        UIImGui();

        explicit UIImGui(vkc::SwapChain& swapChain, const std::shared_ptr<vkc::Device>& device, GLFWwindow* window, VkInstance instance, VkQueue graphicsQueue);

        ~UIImGui();

        void cleanup();

        static void cleanupImGui();

        static void newFrame();

        static void render();

        void resize(vkc::SwapChain& swapChain);

        void cleanupResources();

        void recordCommands(uint32_t imageIndex, VkExtent2D swapChainExtent);

        VkCommandBuffer getCommandBuffer(uint32_t imageIndex);

    private:
        void createRenderPass(VkFormat swapChainFormat);

        void createFrameBuffers(vkc::SwapChain& swapChain);

        void createCommandPool();

        void createCommandBuffers(uint32_t imageCount);

        void createDescriptorPool();

    private:
        VkRenderPass m_renderPass{};
        std::vector<VkFramebuffer> m_framebuffers{};
        VkCommandPool m_commandPool{};
        std::vector<VkCommandBuffer> m_commandBuffers{};
        VkDescriptorPool m_descriptorPool{};
        VkQueue m_graphicsQueue{};
        std::shared_ptr<vkc::Device> m_device{};
    };

} // namepsace core


#endif //PROTOTYPE_ACTION_RPG_UIIMGUI_HPP
