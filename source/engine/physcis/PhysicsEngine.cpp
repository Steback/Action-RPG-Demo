#include "PhysicsEngine.hpp"

#include "glm/glm.hpp"

#include "../Utilities.hpp"
#include "../mesh/Mesh.hpp"
#include "../Application.hpp"
#include "../components/Collision.hpp"


namespace engine {

    PhysicsEngine::PhysicsEngine() {
        broadPhase = new btDbvtBroadphase();
        collisionConfig = new btDefaultCollisionConfiguration();
        dispatcher = new btCollisionDispatcher(collisionConfig);
        solver = new btSequentialImpulseConstraintSolver();
        dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadPhase, solver, collisionConfig);
        dynamicsWorld->setGravity(btVector3(0, -10.0f, 0));
    }

    PhysicsEngine::~PhysicsEngine() = default;

    void PhysicsEngine::cleanup() {
        for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; --i) {
            btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
            btRigidBody* body = btRigidBody::upcast(obj);

            if (body && body->getMotionState())
                delete body->getMotionState();

            if (body && body->getCollisionShape())
                delete body->getCollisionShape();

            dynamicsWorld->removeCollisionObject(obj);

            delete obj;
        }

        delete dynamicsWorld;
        delete solver;
        delete dispatcher;
        delete collisionConfig;
        delete broadPhase;
    }

    void PhysicsEngine::addShape(uint32_t entityID) {
        Entity& entity = Application::m_scene->getEntity(entityID);
        auto& transform = Application::m_scene->getComponent<Transform>(entity.id);
        auto& modelInterface = Application::m_scene->getComponent<ModelInterface>(entity.id);
        auto& collision = Application::m_scene->getComponent<Collision>(entity.id);

        for (auto& node : modelInterface.getNodes()) {
           if (node.mesh > 0) {
               btCollisionShape* shape;

                if (entity.type != EntityType::BUILDING) {
                    shape = new btBoxShape(btVector3(.1, .1, .1));
                } else {
                    shape = new btBoxShape(btVector3(btScalar(50.), btScalar(0.5), btScalar(50.)));
                }

               btVector3 localInertia(0, 0, 0);
               collision.mass = ( entity.type == EntityType::BUILDING ? 0.0f : 1.0f );

               if (collision.mass != 0.0f);
                   shape->calculateLocalInertia(collision.mass, localInertia);

               glm::quat q(transform.getRotation());
               btTransform shapeTransform;
               shapeTransform.setIdentity();
               shapeTransform.setOrigin({transform.getPosition().x, transform.getPosition().y, transform.getPosition().z});
               shapeTransform.setRotation({q.x, q.y, q.z, q.w});

               btRigidBody::btRigidBodyConstructionInfo rbInfo(
                       collision.mass,
                       new btDefaultMotionState(shapeTransform),
                       shape,
                       localInertia
               );

               auto* body = new btRigidBody(rbInfo);
               body->setLinearVelocity(btVector3(0, 0, 0));
               body->setAngularVelocity(btVector3(0, 0, 0));

               if (entity.type == EntityType::BUILDING) {
                   body->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
                   auto* hinge = new btHingeConstraint(*body, btVector3(0, 0, 0), btVector3(0, 1, 0), true);
                   dynamicsWorld->addConstraint(hinge);
               }

               dynamicsWorld->addRigidBody(body);
               collision.rigiBodies.push_back(body);
           }
        }
    }

    void PhysicsEngine::stepSimulation(float deltaTime) {
        dynamicsWorld->stepSimulation(deltaTime);
    }

} // namespace engine