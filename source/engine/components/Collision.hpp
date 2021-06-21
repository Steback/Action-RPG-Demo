#ifndef PROTOTYPE_ACTION_RPG_COLLISION_HPP
#define PROTOTYPE_ACTION_RPG_COLLISION_HPP


#include <vector>

#include "btBulletCollisionCommon.h"


namespace engine {

    class Collision {
    public:
        explicit Collision(uint32_t owner);

        ~Collision();

    public:
        std::vector<btRigidBody*> rigiBodies{};
        uint32_t owner{};
        btScalar mass{10.0f};
    };

} // namespace engine


#endif //PROTOTYPE_ACTION_RPG_COLLISION_HPP
