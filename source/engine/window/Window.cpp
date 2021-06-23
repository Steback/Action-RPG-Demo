#include "Window.hpp"

#include <spdlog/spdlog.h>
#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace engine {

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
        glfwSetCursorPosCallback(m_window, cursorCallback);
        glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
        glfwSetScrollCallback(m_window, scrollCallback);

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

    bool Window::mouseButtonPressed(int mouse) const {
        if (mouse == GLFW_MOUSE_BUTTON_LEFT) {
            return m_lMouseButton;
        } else if (mouse == GLFW_MOUSE_BUTTON_RIGHT) {
            return m_rMouseButton;
        }

        return false;
    }

    glm::vec2 Window::getCursorPos() {
        auto c = m_cursorChangePos;

        m_cursorChangePos = {0.0f, 0.0f};

        return c;
    }

    glm::vec2& Window::getScrollOffset() {
        return m_scrollOffset;
    }

    bool &Window::isScrolling() {
        return m_scrolling;
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

    void Window::cursorCallback(GLFWwindow *window, double xPos, double yPos) {
        auto* w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));

        if (w->m_mouseFirstMove) {
            w->m_lastCursorPos = {static_cast<float>(xPos), static_cast<float>(yPos)};
            w->m_mouseFirstMove = false;
        }

        w->m_cursorChangePos = {static_cast<float>(xPos) - w->m_lastCursorPos.x, static_cast<float>(yPos) - w->m_lastCursorPos.y};
        w->m_lastCursorPos = {static_cast<float>(xPos), static_cast<float>(yPos)};
    }

    void Window::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        auto* w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));

        if (GLFW_MOUSE_BUTTON_LEFT == button && action == GLFW_PRESS) {
            w->m_lMouseButton = true;
        } else if (GLFW_MOUSE_BUTTON_LEFT == button && action == GLFW_RELEASE) {
            w->m_lMouseButton = false;
        }

        if (GLFW_MOUSE_BUTTON_RIGHT == button && action == GLFW_PRESS) {
            w->m_rMouseButton = true;
        } else if (GLFW_MOUSE_BUTTON_RIGHT == button && action == GLFW_RELEASE) {
            w->m_rMouseButton = false;
        }
    }

    void Window::scrollCallback(GLFWwindow *window, double xOffset, double yOffset) {
        auto* w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));

        w->m_scrollOffset = {static_cast<float>(xOffset), static_cast<float>(yOffset)};
        w->m_scrolling = true;
    }

    void Window::setLuaBindings(sol::state& state) {
        sol::table window = state["window"].get_or_create<sol::table>();

        window.new_usertype<WindowSize>("WindowSize",
                                        "width", &WindowSize::width,
                                        "height", &WindowSize::height);

        window.set_function("size", &Window::getSize, this);
    }

} // End namespace core
