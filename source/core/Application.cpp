#include "Application.hpp"

#include "renderer/Tools.hpp"


namespace core {

    std::unique_ptr<core::Renderer> Application::renderer;
    core::ResourceManager* Application::resourceManager;

    Application::Application(const std::string& appName, bool drawGrid) {
        spdlog::info("[App] Start");

        m_window = std::make_unique<core::Window>(appName, 1776, 1000);

        VkApplicationInfo appInfo = vk::initializers::applicationInfo();
        appInfo.pApplicationName = appName.c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
        appInfo.pEngineName = "Custom Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
        appInfo.apiVersion = VK_API_VERSION_1_2;

        std::vector<const char*> enabledExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        m_instance.init(appInfo);

        VkPhysicalDevice physicalDevice;
        m_instance.createSurface(m_window->getWindow(), m_surface);
        m_instance.pickPhysicalDevice(physicalDevice, m_surface, enabledExtensions);

        m_device = new vk::Device(physicalDevice);

        VkPhysicalDeviceFeatures deviceFeatures{};
        m_device->createLogicalDevice(deviceFeatures, enabledExtensions);

        renderer = std::make_unique<core::Renderer>(m_window, *m_instance, appName, m_device, m_surface);
        resourceManager = new core::ResourceManager(m_device, renderer->getGraphicsQueue());

        renderer->init(drawGrid);

        m_scene = std::make_unique<core::Scene>();

        spdlog::info("[App] Start");
    }

    Application::~Application() = default;

    void Application::run() {
        init();
        loop();
        shutdown();
    }

    void Application::render() {
        renderer->acquireNextImage();
        renderer->beginRenderPass();
        {
            renderer->setPipeline();
            m_scene->render();
            renderer->drawGrid();
            draw();
        }
        renderer->endRenderPass();
        renderer->drawFrame();
    }

    void Application::shutdown() {
        vkDeviceWaitIdle(m_device->m_logicalDevice);

        cleanup();
        m_scene->cleanup();
        renderer->cleanup();
        resourceManager->cleanup();
        delete resourceManager;
        m_device->destroy();
        delete m_device;
        m_instance.destroySurface(m_surface);
        m_instance.destroy();
        m_window->clean();

        spdlog::info("[App] Cleaned");
    }

    void Application::loop() {
        while (m_window->isOpen()) {
            glfwPollEvents();

            auto now = static_cast<float>(glfwGetTime());
            m_deltaTime = now - m_lastTime;
            m_lastTime = now;

            update();
            render();
        }
    }

} // namespace core