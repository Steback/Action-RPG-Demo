#include <memory>

#include "GLFW/glfw3.h"

#include "logger/Logger.hpp"
#include "Editor.hpp"

int main() {
    core::Logger logger;
    logger.init("App", "error.log");

    editor::Editor editor;

    try {
        editor.run();
    } catch (const std::exception& ex) {
        logger.sendLog(core::ERROR, ex.what());
    }

    return EXIT_SUCCESS;
}

