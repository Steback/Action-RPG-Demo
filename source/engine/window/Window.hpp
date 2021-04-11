#ifndef PROTOTYPE_ACTION_RPG_WINDOW_HPP
#define PROTOTYPE_ACTION_RPG_WINDOW_HPP


#include <string>
#include <array>

#include "GLFW/glfw3.h"
#include "glm/glm.hpp"


namespace engine {
    struct WindowSize {
        uint32_t width{}, height{};
    };

    class Window {
    public:
        Window(const std::string& name, int width, int height);

        ~Window();

        bool isOpen();

        void clean();

        WindowSize getSize();

        [[nodiscard]] float aspect() const;

        GLFWwindow* getWindow();

        bool& resize();

        std::array<char, 1024>& getKeys();

        bool keyPressed(int key);

        void setKeyValue(int key, bool pressed);

        bool mouseButtonPressed(int button);

        glm::vec2 getCursorPos();

        glm::vec2& getScrollOffset();

        bool& isScrolling();

    private:
        static void framebufferResizeCallback(GLFWwindow* tWindow, int width, int height);

        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

        static void cursorCallback(GLFWwindow* window, double xPos, double yPos);

        static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

        static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);

    private:
        GLFWwindow* m_window{};
        WindowSize m_size;
        bool m_resize = false;
        std::array<char, 1024> m_keys{false};
        glm::vec2 m_cursorChangePos{};
        glm::vec2 m_lastCursorPos{};
        bool m_rMouseButton{};
        bool m_lMouseButton{};
        glm::vec2 m_scrollOffset{};
        bool m_mouseFirstMove{};
        bool m_scrolling{};
    };

} // End namespace core


#endif //PROTOTYPE_ACTION_RPG_WINDOW_HPP
