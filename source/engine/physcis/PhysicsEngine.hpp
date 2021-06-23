#ifndef PROTOTYPE_ACTION_RPG_PHYSICSENGINE_HPP
#define PROTOTYPE_ACTION_RPG_PHYSICSENGINE_HPP


#include <memory>
#include <vector>
#include <unordered_map>

#include "btBulletDynamicsCommon.h"


namespace engine {

    struct Vertex;

    class Collision;

    class PhysicsEngine {
    public:
        PhysicsEngine();

        ~PhysicsEngine();

        void cleanup();

        void addShape(uint32_t entityID);

        void stepSimulation(float deltaTime);

    private:
        btDiscreteDynamicsWorld* dynamicsWorld{};
        btBroadphaseInterface* broadPhase{};
        btDefaultCollisionConfiguration* collisionConfig{};
        btCollisionDispatcher* dispatcher{};
        btSequentialImpulseConstraintSolver* solver{};
    };

} // namespace engine


#endif //PROTOTYPE_ACTION_RPG_PHYSICSENGINE_HPP
