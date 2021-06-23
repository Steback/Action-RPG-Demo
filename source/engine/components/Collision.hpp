#ifndef PROTOTYPE_ACTION_RPG_COLLISION_HPP
#define PROTOTYPE_ACTION_RPG_COLLISION_HPP


#include <vector>

#include "btBulletCollisionCommon.h"

#include "Transform.hpp"


namespace engine {

    class Collision {
    public:
        explicit Collision(uint32_t owner);

        ~Collision();

        void update(Transform* transform);

    public:
        std::vector<btRigidBody*> rigiBodies{};
        uint32_t owner{};
        btScalar mass{};
    };

} // namespace engine


#endif //PROTOTYPE_ACTION_RPG_COLLISION_HPP
