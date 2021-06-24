#ifndef PROTOTYPE_ACTION_RPG_COLLISION_HPP
#define PROTOTYPE_ACTION_RPG_COLLISION_HPP


#include <vector>

#include "btBulletCollisionCommon.h"

#include "Transform.hpp"


namespace engine {

    class Collision {
    public:
        Collision(uint32_t owner, float mass, const glm::vec3& halfSize);

        ~Collision();

        void update(Transform* worldTransform);

    public:
        std::vector<btRigidBody*> rigiBodies{};
        uint32_t owner{};
        btScalar mass{};
        btVector3 halfSize{};
        btTransform transform;
    };

} // namespace engine


#endif //PROTOTYPE_ACTION_RPG_COLLISION_HPP
