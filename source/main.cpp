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

        mWindow = std::make_unique<core::Window>("Prototype Action RPG", 1200, 700);
        mRender = std::make_unique<core::Renderer>(mWindow);

        spdlog::info("[App] Initialized");
    }

    void loop() {
        while (mWindow->isOpen()) {
            glfwPollEvents();
        }
    }

    void clean() {
        mRender->clean();
        mWindow->clean();

        spdlog::info("[App] Cleaned");
    }

private:
    std::unique_ptr<core::Window> mWindow;
    std::unique_ptr<core::Renderer> mRender;
};


int main() {
    core::Logger logger;
    logger.init("logger", "error.log");

    App app;

    try {
        app.run();
    } catch (const spdlog::spdlog_ex& ex) {
        logger.sendLog(core::ERROR, ex.what());
    }

    return EXIT_SUCCESS;
}