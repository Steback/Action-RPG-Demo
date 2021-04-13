#include "Window.hpp"

#include <utility>

#include "imgui.h"


namespace engine::ui {

    Window::Window(WINDOW_ARGS) : m_name(std::move(name)), m_width(with), m_height(height), m_flags(flags) {

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

    void Window::draw(const sol::function& f) {
        ImGui::SetNextWindowSize({m_width, m_height});
        ImGui::Begin(m_name.c_str(), (m_flags & WindowFlags::reqOpen ? &m_open : nullptr), 0);
        {
            f();
        }
        ImGui::End();
    }

    bool Window::isOpen() const {
        return m_open;
    }

    void Window::setLuaClass(sol::table& state) {
        sol::table flags = state.new_enum("WindowFlags",
                                          "reqOpen", WindowFlags::reqOpen);

        state.new_usertype<Window>("Window",
                                   sol::call_constructor, sol::factories([](WINDOW_ARGS){ return Window(std::move(name), with, height, flags); }),
                                   "getName", &Window::getName,
                                   "setName", &Window::setName,
                                   "getWidth", &Window::getWidth,
                                   "setWidth", &Window::setWidth,
                                   "getHeight", &Window::getHeight,
                                   "setHeight", &Window::setHeight,
                                   "draw", &Window::draw,
                                   "open", &Window::isOpen);
    }

} //namespace engine::ui