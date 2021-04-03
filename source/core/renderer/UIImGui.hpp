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

        explicit UIImGui(vkc::SwapChain& swapChain, const std::shared_ptr<vkc::Device>& device, GLFWwindow* window, vk::Instance instance, vk::Queue graphicsQueue);

        ~UIImGui();

        void cleanup();

        static void cleanupImGui();

        static void newFrame();

        static void render();

        void resize(vkc::SwapChain& swapChain);

        void cleanupResources();

        void recordCommands(uint32_t imageIndex, vk::Extent2D swapChainExtent);

        vk::CommandBuffer getCommandBuffer(uint32_t imageIndex);

    private:
        void createRenderPass(vk::Format swapChainFormat);

        void createFrameBuffers(vkc::SwapChain& swapChain);

        void createCommandPool();

        void createCommandBuffers(uint32_t imageCount);

        void createDescriptorPool();

    private:
        vk::RenderPass m_renderPass{};
        std::vector<vk::Framebuffer> m_framebuffers{};
        vk::CommandPool m_commandPool{};
        std::vector<vk::CommandBuffer> m_commandBuffers{};
        vk::DescriptorPool m_descriptorPool{};
        vk::Queue m_graphicsQueue{};
        std::shared_ptr<vkc::Device> m_device{};
    };

} // namepsace core


#endif //PROTOTYPE_ACTION_RPG_UIIMGUI_HPP
