#include "Window.hpp"

#include <spdlog/spdlog.h>


namespace core {

    Window::Window(const std::string& name, int width, int height) {
        if (!glfwInit()) {
            spdlog::throw_spdlog_ex("[GLFW] Failed to init.");
        }

        m_size = {
            .width = static_cast<uint32_t>(width),
            .height = static_cast<uint32_t>(height)
        };

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);

        glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);

        spdlog::info("[Window] Initialized");
    }

    Window::~Window() = default;

    bool Window::isOpen() {
        return !glfwWindowShouldClose(m_window);
    }

    void Window::clean() {
        glfwDestroyWindow(m_window);
        glfwTerminate();

        spdlog::info("[Window] Cleaned");
    }

    WindowSize Window::getSize() {
        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);

        m_size.width = static_cast<uint32_t>(width);
        m_size.height = static_cast<uint32_t>(height);

        return m_size;
    }

    void Window::framebufferResizeCallback(GLFWwindow *tWindow, int width, int height) {
        auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(tWindow));
        window->m_resize = true;
    }

    float Window::aspect() const {
        return static_cast<float>(m_size.width) / static_cast<float>(m_size.height);
    }

    GLFWwindow *Window::getWindow() {
        return m_window;
    }

    bool& Window::resize() {
        return m_resize;
    }

} // End namespace core
