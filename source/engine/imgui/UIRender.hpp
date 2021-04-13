#ifndef PROTOTYPE_ACTION_RPG_UIRENDER_HPP
#define PROTOTYPE_ACTION_RPG_UIRENDER_HPP


#include <vector>
#include <memory>

#include "vulkan/vulkan.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

#include "Window.hpp"
#include "../renderer/SwapChain.hpp"
#include "../renderer/Device.hpp"


namespace engine {

    class CommandList;

    class UIRender {
    public:
        UIRender();

        explicit UIRender(engine::SwapChain& swapChain, const std::shared_ptr<engine::Device>& device, GLFWwindow* window,
                         vk::Instance instance, vk::Queue graphicsQueue, std::shared_ptr<CommandList> commandList);

        ~UIRender();

        void cleanup();

        static void cleanupImGui();

        static void newFrame();

        static void render();

        void resize(engine::SwapChain& swapChain);

        void cleanupResources();

        void recordCommands(uint32_t imageIndex, vk::Extent2D swapChainExtent);

        void setLuaBindings(sol::state& state);

        engine::ui::Window creteWindow(const std::string& name, float with, float height, uint32_t flags);

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
        std::vector<engine::ui::Window> m_windows;
    };

} // namepsace core


#endif //PROTOTYPE_ACTION_RPG_UIRENDER_HPP
