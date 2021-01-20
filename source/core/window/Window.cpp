#include "Window.hpp"

#include <spdlog/spdlog.h>


namespace core {

    Window::Window(const std::string& name, int width, int height) : mWidth(0), mHeight(0) {
        if (!glfwInit()) {
            spdlog::throw_spdlog_ex("[GLFW] Failed to init.");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        mWindow = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);

        spdlog::info("[Window] Initialized");
    }

    Window::~Window() = default;

    bool Window::isOpen() {
        return !glfwWindowShouldClose(mWindow);
    }

    void Window::clean() {
        glfwDestroyWindow(mWindow);
        glfwTerminate();

        spdlog::info("[Window] Cleaned");
    }

} // End namespace core
