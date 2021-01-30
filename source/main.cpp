#include <memory>

#include "GLFW/glfw3.h"

#include "core/logger/Logger.hpp"
#include "core/window/Window.hpp"
#include "core/renderer/Renderer.hpp"


class App {
public:
    App() = default;

    ~App() = default;

    void run() {
        init();
        loop();
        clean();
    }

    void init() {
        spdlog::info("[App] Start");

        m_window = std::make_unique<core::Window>("Prototype Action RPG", 1200, 700);
        m_renderer = std::make_unique<core::Renderer>(m_window);

        spdlog::info("[App] Initialized");
    }

    void loop() {
        while (m_window->isOpen()) {
            glfwPollEvents();
            m_renderer->draw();
        }
    }

    void clean() {
        m_renderer->cleanup();
        m_window->clean();

        spdlog::info("[App] Cleaned");
    }

private:
    std::unique_ptr<core::Window> m_window;
    std::unique_ptr<core::Renderer> m_renderer;
};


int main() {
    core::Logger logger;
    logger.init("App", "error.log");

    App app;

    try {
        app.run();
    } catch (const std::exception& ex) {
        logger.sendLog(core::ERROR, ex.what());
    }

    return EXIT_SUCCESS;
}