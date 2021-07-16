#include "Application.hpp"

#include "spdlog/spdlog.h"

#include "renderer/CommandList.hpp"
#include "renderer/GraphicsPipeline.hpp"
#include "lua/MathBindings.hpp"
#include "physcis/PhysicsEngine.hpp"


namespace engine {

    std::unique_ptr<engine::RenderEngine> Application::m_renderer;
    std::unique_ptr<engine::ResourceManager> Application::m_resourceManager;
    std::unique_ptr<engine::Scene> Application::m_scene;
    std::unique_ptr<ThreadPool> Application::m_threadPool;
    std::unique_ptr<PhysicsEngine> Application::physicsEngine;
    std::unique_ptr<MousePicking> Application::mousePicking;
    bool Application::m_editor;
    const char* Application::keys;

    Application::Application(const std::string& appName, const glm::vec4& clearColor, bool editor)
            : m_clearColor(clearColor) {
        spdlog::info("[App] Start");
        m_editor = editor;

        m_window = std::make_shared<engine::Window>(appName, 1776, 1000);
        keys = m_window->getKeys().data();

        m_instance = std::make_shared<engine::Instance>(vk::ApplicationInfo{
            .pApplicationName = appName.c_str(),
            .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
            .pEngineName = "Custom Engine",
            .engineVersion = VK_MAKE_VERSION(0, 1, 0)
        });

        m_device = std::make_shared<engine::Device>(m_instance);
        m_renderer = std::make_unique<engine::RenderEngine>(m_window, m_instance->getInstance(), appName, m_device, m_instance->createSurface(m_window->getWindow()));
        m_resourceManager = std::make_unique<engine::ResourceManager>(m_device, m_renderer->getGraphicsQueue());

        vk::PushConstantRange constantRange{
                .stageFlags = vk::ShaderStageFlagBits::eVertex,
                .offset = 0,
                .size = sizeof(MVP)
        };

        std::string vertShader = "model.vert.spv";
        std::string fragShader = "model.frag.spv";
        m_pipelineAnimation = m_renderer->addPipeline(Application::m_resourceManager->createShader(vertShader, fragShader, {constantRange}),
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

        physicsEngine = std::make_unique<PhysicsEngine>();

        mousePicking = std::make_unique<MousePicking>(m_window);
        mousePicking->setLuaBindings(m_luaManager.getState());

        m_threadPool = std::make_unique<ThreadPool>();

        spdlog::info("[App] Start");
    }

    Application::~Application() = default;

    void Application::run() {
        init();

        updatePipeline();

        m_resourceManager->initialPose();
        loop();
        shutdown();
    }

    void Application::shutdown() {
        m_device->m_logicalDevice.waitIdle();

        cleanup();
        m_threadPool->stop();
        m_scene->cleanup();
        physicsEngine->cleanup();
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
            physicsEngine->stepSimulation(m_deltaTime);
            mousePicking->pick();
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
                    m_pipelineAnimation->bind(m_commands->getBuffer());
                    m_commands->getBuffer().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineAnimation->getLayout(), 0, 1,
                                                               &m_renderer->getDescriptorSet(), 0, nullptr);
                    m_scene->render(m_commands->getBuffer(), m_pipelineAnimation);
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

    void Application::updatePipeline() {
        uint32_t maxPoolSize = m_resourceManager->getMeshesCount();
        vk::DescriptorPoolSize poolSizes = {
            .type = vk::DescriptorType::eUniformBuffer,
            .descriptorCount = maxPoolSize
        };

        m_resourceManager->createMeshDescriptors({poolSizes}, maxPoolSize + 1);
        m_resourceManager->createMeshDescriptorSets();

        std::vector<vk::DescriptorSetLayout> layouts = {
                m_renderer->getDescriptorSetLayout(),
                m_resourceManager->getTextureDescriptorSetLayout(),
                m_resourceManager->getMeshDescriptorSetLayout()
        };

        m_pipelineAnimation->cleanup();
        m_pipelineAnimation->create(layouts, m_renderer->getSwapChain(), m_renderer->getRenderPass(), m_device->getMaxUsableSampleCount());
    }

} // namespace core