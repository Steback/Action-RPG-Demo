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
        glfwSetKeyCallback(m_window, keyCallback);

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

    float Window::aspect() const {
        return static_cast<float>(m_size.width) / static_cast<float>(m_size.height);
    }

    GLFWwindow *Window::getWindow() {
        return m_window;
    }

    bool& Window::resize() {
        return m_resize;
    }

    std::array<char, 1024> &Window::getKeys() {
        return m_keys;
    }

    bool Window::keyPressed(int key) {
        return m_keys[key];
    }

    void Window::setKeyValue(int key, bool pressed) {
        m_keys[key] = pressed;
    }

    void Window::framebufferResizeCallback(GLFWwindow *tWindow, int width, int height) {
        auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(tWindow));
        window->m_resize = true;
    }

    void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto* w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));

        if (action >= 0 && key <= 1024) {
            if (action == GLFW_PRESS) {
                w->m_keys[key] = true;
            } else if (action == GLFW_RELEASE) {
                w->m_keys[key] = false;
            }
        }
    }

} // End namespace core
