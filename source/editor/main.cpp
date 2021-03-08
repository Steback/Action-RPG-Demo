#include <memory>

#include "Editor.hpp"

int main() {
    editor::Editor editor;

    try {
        editor.run();
    } catch (const std::exception& ex) {
        spdlog::error("{}", ex.what());
    }

    return EXIT_SUCCESS;
}

