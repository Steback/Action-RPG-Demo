#ifndef PROTOTYPE_ACTION_RPG_WINDOW_HPP
#define PROTOTYPE_ACTION_RPG_WINDOW_HPP


#include <string>

#include "GLFW/glfw3.h"


namespace core {

    class Window {
    public:
        Window();

        ~Window();

        void init(const std::string& name, int width, int height);

        bool isOpen();

        void clean();

    private:
        GLFWwindow* mWindow{};
        int mWidth, mHeight;
    };

} // End namespace core


#endif //PROTOTYPE_ACTION_RPG_WINDOW_HPP
