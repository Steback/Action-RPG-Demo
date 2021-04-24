#include "Window.hpp"

#include <utility>

#include "imgui.h"
#include "fmt/format.h"

#include "../Constants.hpp"


namespace engine::ui {

    Window::Window(std::string name, float with, float height, uint32_t flags)
            : m_name(std::move(name)), m_width(with), m_height(height), m_flags(flags) {

    }

    std::string Window::getName() {
        return m_name;
    }

    void Window::setName(const std::string& name) {
        m_name = name;
    }

    float Window::getWidth() const {
        return m_width;
    }

    void Window::setWidth(float width) {
        m_width = width;
    }

    float Window::getHeight() const {
        return m_height;
    }

    void Window::setHeight(float height) {
        m_height = height;
    }

    void Window::drawLua(const sol::function& f, int flags) {
        if (m_flags & WindowFlags::fixPosition)
            ImGui::SetNextWindowPos({m_position.x, m_position.y});

        ImGui::SetNextWindowSize({m_width, m_height});
        ImGui::Begin(m_name.c_str(), (m_flags & WindowFlags::close ? &m_open : nullptr), flags);
        {
#ifdef CORE_DEBUG
        const sol::protected_function& func = f;
        sol::protected_function_result result = func();

        if (!result.valid()) {
            sol::error error = result;
            fmt::print("{}\n", error.what());
        }
#else
            f();
#endif

        }
        ImGui::End();
    }

    void Window::draw(std::function<void()> f, int flags) {
        if (m_flags & WindowFlags::fixPosition)
            ImGui::SetNextWindowPos({m_position.x, m_position.y});

        ImGui::SetNextWindowSize({m_width, m_height});
        ImGui::Begin(m_name.c_str(), (m_flags & WindowFlags::close ? &m_open : nullptr), flags);
        {
            f();
        }
        ImGui::End();
    }

    bool Window::isOpen() const {
        return m_open;
    }

    void Window::setState(bool state) {
        m_open = state;
    }

    void Window::setPosition(float x, float y) {
        m_position = {x, y};
    }

    void Window::setLuaClass(sol::table& state) {
        sol::table flags = state.new_enum("WindowFlags",
                                          "close", WindowFlags::close,
                                          "fixPosition", WindowFlags::fixPosition);

        state.new_usertype<Window>("Window",
                                   sol::call_constructor, sol::factories([](std::string name, float with, float height, uint32_t flags){ return Window(std::move(name), with, height, flags); }),
                                   "getName", &Window::getName,
                                   "setName", &Window::setName,
                                   "getWidth", &Window::getWidth,
                                   "setWidth", &Window::setWidth,
                                   "getHeight", &Window::getHeight,
                                   "setHeight", &Window::setHeight,
                                   "draw", &Window::drawLua,
                                   "open", &Window::isOpen,
                                   "setState", &Window::setState,
                                   "setPosition", &Window::setPosition);
    }

} //namespace engine::ui