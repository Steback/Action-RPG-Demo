#include "Application.hpp"

#include "spdlog/spdlog.h"

#include "renderer/CommandList.hpp"
#include "renderer/GraphicsPipeline.hpp"


namespace core {

    std::unique_ptr<core::RenderDevice> Application::m_renderer;
    std::unique_ptr<core::ResourceManager> Application::m_resourceManager;
    std::unique_ptr<core::Scene> Application::m_scene;

    Application::Application(const std::string& appName, const glm::vec4& clearColor)
            : m_clearColor(clearColor) {
        spdlog::info("[App] Start");

        m_window = std::make_shared<core::Window>(appName, 1776, 1000);

        m_instance = std::make_shared<core::Instance>(vk::ApplicationInfo{
            .pApplicationName = appName.c_str(),
            .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
            .pEngineName = "Custom Engine",
            .engineVersion = VK_MAKE_VERSION(0, 1, 0)
        });

        m_device = std::make_shared<core::Device>(m_instance);
        m_renderer = std::make_unique<core::RenderDevice>(m_window, m_instance->getInstance(), appName, m_device, m_instance->createSurface(m_window->getWindow()));
        m_resourceManager = std::make_unique<core::ResourceManager>(m_device, m_renderer->getGraphicsQueue());

        vk::PushConstantRange constantRange{
            .stageFlags = vk::ShaderStageFlagBits::eVertex,
            .offset = 0,
            .size = sizeof(MVP)
        };

        m_pipeline = m_renderer->addPipeline(core::Application::m_resourceManager->createShader("model.vert.spv", "model.frag.spv", {constantRange}),
                                             m_device->m_logicalDevice);
        m_renderer->init();

        m_scene = std::make_unique<core::Scene>();
        m_commands = m_renderer->addCommandList();

        spdlog::info("[App] Start");
    }

    Application::~Application() = default;

    void Application::run() {
        init();
        loop();
        shutdown();
    }

    void Application::shutdown() {
        vkDeviceWaitIdle(m_device->m_logicalDevice);

        cleanup();
        m_scene->cleanup();
        m_renderer->cleanup(m_instance);
        m_resourceManager->cleanup();
        m_device->destroy();
        m_instance->destroy();
        m_window->clean();

        spdlog::info("[App] Cleaned");
    }

    void Application::loop() {
        while (m_window->isOpen()) {
            glfwPollEvents();

            auto now = static_cast<float>(glfwGetTime());
            m_deltaTime = now - m_lastTime;
            m_lastTime = now;

            m_renderer->updateVP(m_scene->getCamera().getView(), m_scene->getCamera().getProjection(m_window->aspect()));
            m_scene->update(m_deltaTime);

            update();

            core::UIImGui::newFrame();
            drawUI();
            core::UIImGui::render();

            m_renderer->acquireNextImage();
            m_commands->begin();
            {
                m_commands->beginRenderPass(m_renderer->getRenderPass(), m_clearColor, m_renderer->getFrameBuffer(), m_renderer->getSwapChainExtent());
                {
                    m_pipeline->bind(m_commands->getBuffer());
                    m_scene->render(m_commands->getBuffer(), m_pipeline->getLayout(), m_renderer->getDescriptorSet(), m_renderer->m_mvp);
                    renderCommands(m_commands->getBuffer());
                }
                m_commands->endRenderPass();
            }
            m_commands->end();
            m_renderer->render();
        }
    }

} // namespace core