#include "GLFW/glfw3.h"

#include "core/logger/Logger.hpp"
#include "core/window/Window.hpp"


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
        mWindow.init("Prototype Action RPG", 1200, 700);
    }

    void loop() {
        while (mWindow.isOpen()) {
            glfwPollEvents();
        }
    }

    void clean() {
        mWindow.clean();
    }

private:
    core::Window mWindow;
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