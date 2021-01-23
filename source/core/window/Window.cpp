#include "Window.hpp"

#include <spdlog/spdlog.h>


namespace core {

    Window::Window(const std::string& name, int width, int height) {
        if (!glfwInit()) {
            spdlog::throw_spdlog_ex("[GLFW] Failed to init.");
        }

        mSize = {
            .mWidth = static_cast<uint32_t>(width),
            .mHeight = static_cast<uint32_t>(height)
        };

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        mWindow = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);

        glfwSetWindowUserPointer(mWindow, this);
        glfwSetFramebufferSizeCallback(mWindow, framebufferResizeCallback);

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

    WindowSize Window::getSize() {
        int width, height;
        glfwGetFramebufferSize(mWindow, &width, &height);

        mSize.mWidth = static_cast<uint32_t>(width);
        mSize.mHeight = static_cast<uint32_t>(height);

        return mSize;
    }

    void Window::framebufferResizeCallback(GLFWwindow *tWindow, int width, int height) {
        auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(tWindow));
        window->mResize = true;
    }

} // End namespace core
