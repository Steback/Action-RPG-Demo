#ifndef ACTION_RPG_DEMO_MOVEMENT_HPP
#define ACTION_RPG_DEMO_MOVEMENT_HPP


#include "GLFW/glfw3.h"


namespace engine {

    class Transform;
    class AnimationInterface;

    class Movement {
    public:
        enum Key {
            UP = GLFW_KEY_W,
            DOWN = GLFW_KEY_S,
            LEFT = GLFW_KEY_A,
            RIGHT = GLFW_KEY_D
        };

    public:
        Movement();

        void update(float deltaTime, Transform* transform, AnimationInterface* animation);

    public:
        bool isMoving{};
    };

}


#endif //ACTION_RPG_DEMO_MOVEMENT_HPP
