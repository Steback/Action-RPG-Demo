#ifndef PROTOTYPE_ACTION_RPG_WINDOW_HPP
#define PROTOTYPE_ACTION_RPG_WINDOW_HPP


#include <string>

#include "GLFW/glfw3.h"


class Window {
public:
    Window(const std::string& name, int mWidth, int mHeight);

    virtual ~Window();

private:
    GLFWwindow* mWindow;
    int mWidth, mHeight;
};


#endif //PROTOTYPE_ACTION_RPG_WINDOW_HPP
