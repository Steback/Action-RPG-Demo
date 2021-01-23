#ifndef PROTOTYPE_ACTION_RPG_WINDOW_HPP
#define PROTOTYPE_ACTION_RPG_WINDOW_HPP


#include <string>

#include "GLFW/glfw3.h"


namespace core {
    struct WindowSize {
        uint32_t mWidth{}, mHeight{};
    };


    class Window {
        friend class Renderer;

    public:
        Window(const std::string& name, int width, int height);

        ~Window();

        bool isOpen();

        void clean();

        WindowSize getSize();

    private:
        static void framebufferResizeCallback(GLFWwindow* tWindow, int width, int height);

    private:
        GLFWwindow* mWindow{};
        WindowSize mSize;
        bool mResize = false;
    };

} // End namespace core


#endif //PROTOTYPE_ACTION_RPG_WINDOW_HPP
