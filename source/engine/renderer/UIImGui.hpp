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


namespace engine {

    class CommandList;

    class UIImGui {
    public:
        UIImGui();

        explicit UIImGui(engine::SwapChain& swapChain, const std::shared_ptr<engine::Device>& device, GLFWwindow* window,
                         vk::Instance instance, vk::Queue graphicsQueue, std::shared_ptr<CommandList> commandList);

        ~UIImGui();

        void cleanup();

        static void cleanupImGui();

        static void newFrame();

        static void render();

        void resize(engine::SwapChain& swapChain);

        void cleanupResources();

        void recordCommands(uint32_t imageIndex, vk::Extent2D swapChainExtent);

    private:
        void createRenderPass(vk::Format swapChainFormat);

        void createFrameBuffers(engine::SwapChain& swapChain);

        void createDescriptorPool();

    private:
        vk::RenderPass m_renderPass{};
        std::shared_ptr<CommandList> m_commands;
        std::vector<vk::Framebuffer> m_framebuffers{};
        vk::DescriptorPool m_descriptorPool{};
        vk::Device m_device{};
    };

} // namepsace core


#endif //PROTOTYPE_ACTION_RPG_UIIMGUI_HPP
