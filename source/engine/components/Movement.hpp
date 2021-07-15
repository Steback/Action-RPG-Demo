#ifndef ACTION_RPG_DEMO_MOVEMENT_HPP
#define ACTION_RPG_DEMO_MOVEMENT_HPP


#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"


namespace engine {

    class Transform;
    class AnimationInterface;

    class Movement {
    public:
        explicit Movement(Transform& transform);

        void update(float deltaTime, Transform* transform, AnimationInterface* animation);

    public:
        bool isMoving{};
        glm::vec3 moveTo{};
        glm::vec3 direction{};
        bool isActive{true};
    };

}


#endif //ACTION_RPG_DEMO_MOVEMENT_HPP
