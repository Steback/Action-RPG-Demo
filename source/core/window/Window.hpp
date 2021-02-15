#ifndef PROTOTYPE_ACTION_RPG_WINDOW_HPP
#define PROTOTYPE_ACTION_RPG_WINDOW_HPP


#include <string>
#include <array>

#include "GLFW/glfw3.h"


namespace core {
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

    private:
        static void framebufferResizeCallback(GLFWwindow* tWindow, int width, int height);

        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    private:
        GLFWwindow* m_window{};
        WindowSize m_size;
        bool m_resize = false;
        std::array<char, 1024> m_keys{false};
    };

} // End namespace core


#endif //PROTOTYPE_ACTION_RPG_WINDOW_HPP
