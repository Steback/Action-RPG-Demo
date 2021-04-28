#include "Application.hpp"

#include "spdlog/spdlog.h"

#include "renderer/CommandList.hpp"
#include "renderer/GraphicsPipeline.hpp"
#include "lua/MathBindings.hpp"


namespace engine {

    std::unique_ptr<engine::RenderDevice> Application::m_renderer;
    std::unique_ptr<engine::ResourceManager> Application::m_resourceManager;
    std::unique_ptr<engine::Scene> Application::m_scene;
    std::unique_ptr<ThreadPool> Application::m_threadPool;

    Application::Application(const std::string& appName, const glm::vec4& clearColor)
            : m_clearColor(clearColor) {
        spdlog::info("[App] Start");

        m_window = std::make_shared<engine::Window>(appName, 1776, 1000);

        m_instance = std::make_shared<engine::Instance>(vk::ApplicationInfo{
            .pApplicationName = appName.c_str(),
            .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
            .pEngineName = "Custom Engine",
            .engineVersion = VK_MAKE_VERSION(0, 1, 0)
        });

        m_device = std::make_shared<engine::Device>(m_instance);
        m_renderer = std::make_unique<engine::RenderDevice>(m_window, m_instance->getInstance(), appName, m_device, m_instance->createSurface(m_window->getWindow()));
        m_resourceManager = std::make_unique<engine::ResourceManager>(m_device, m_renderer->getGraphicsQueue());

        vk::PushConstantRange constantRange{
            .stageFlags = vk::ShaderStageFlagBits::eVertex,
            .offset = 0,
            .size = sizeof(glm::mat4)
        };

        m_pipeline = m_renderer->addPipeline(engine::Application::m_resourceManager->createShader("model.vert.spv", "model.frag.spv", {constantRange}),
                                             m_device->m_logicalDevice);
        m_renderer->init();

        m_scene = std::make_unique<engine::Scene>();
        m_commands = m_renderer->addCommandList();

        m_ui = engine::UIRender(m_renderer->getSwapChain(), m_device, m_window->getWindow(), m_instance->getInstance(), m_renderer->getGraphicsQueue(),
                               m_renderer->addCommandList());

        m_window->setLuaBindings(m_luaManager.getState());
        lua::setMathBindings(m_luaManager.getState());
        m_ui.setLuaBindings(m_luaManager.getState());
        m_scene->setLuaBindings(m_luaManager.getState());

        sol::table tools = m_luaManager.getState()["tools"].get_or_create<sol::table>();
        tools.set_function("hashString", &tools::hashString);
        tools.set_function("getDeltaTime", &Application::getDeltaTime, this);

        m_threadPool = std::make_unique<ThreadPool>();

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
        m_threadPool->stop();
        m_scene->cleanup();
        m_ui.cleanupResources();
        m_ui.cleanup();
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

            engine::UIRender::newFrame();
            drawUI();
            m_luaManager.executeFunction("drawUI");
            engine::UIRender::render();

            m_renderer->acquireNextImage();
            m_commands->begin();
            {
                m_commands->beginRenderPass(m_renderer->getRenderPass(), m_clearColor, m_renderer->getFrameBuffer(), m_renderer->getSwapChainExtent());
                {
                    m_pipeline->bind(m_commands->getBuffer());
                    m_scene->render(m_commands->getBuffer(), m_pipeline);
                    renderCommands(m_commands->getBuffer());
                }
                m_commands->endRenderPass();
            }
            m_commands->end();
            m_ui.recordCommands(m_renderer->getImageIndex(), m_renderer->getSwapChainExtent());
            m_renderer->render();
        }
    }

    float Application::getDeltaTime() const {
        return m_deltaTime;
    }

} // namespace core