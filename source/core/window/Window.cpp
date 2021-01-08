#include "Window.hpp"

#include <spdlog/spdlog.h>


namespace core {

    Window::Window() : mWidth(0), mHeight(0) {

    }

    Window::~Window() = default;

    void Window::init(const std::string &name, int width, int height) {
        if (!glfwInit()) {
            throw spdlog::spdlog_ex("[GLFW] Failed to init.");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        mWindow = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    }

    bool Window::isOpen() {
        return !glfwWindowShouldClose(mWindow);
    }

    void Window::clean() {
        glfwDestroyWindow(mWindow);
        glfwTerminate();
    }

} // End namespace core
