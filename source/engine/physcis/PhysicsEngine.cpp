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
        dynamicsWorld->setGravity({0, -9.0f, 0});
    }

    PhysicsEngine::~PhysicsEngine() = default;

    void PhysicsEngine::cleanup() {
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
               Mesh& mesh = Application::m_resourceManager->getMesh(node.mesh);
               std::vector<Vertex> vertices = mesh.getVertices();
               std::vector<uint32_t> indices = mesh.getIndices();
               std::vector<btVector3> positions(vertices.size());
               std::vector<int> btIndices(indices.size());

               for (int i = 0; i < vertices.size(); ++i)
                   positions[i] = btVector3(vertices[i].position.x, vertices[i].position.y, vertices[i].position.z);

               for (int i = 0; i < indices.size(); ++i)
                   btIndices[i] = static_cast<int>(indices[i]);

               auto* indexVertexArray = new btTriangleIndexVertexArray(
                       (int)btIndices.size() / 3,
                       btIndices.data(),
                       3 * sizeof(int),
                       (int)positions.size(),
                       (btScalar*)&positions[0].x(),
                       sizeof(btVector3)
               );

               bool useQuantizedAabbCompression = true;
               shape = new btBvhTriangleMeshShape(indexVertexArray, useQuantizedAabbCompression);
               shape->setLocalScaling(btVector3(transform.getSize().x, transform.getSize().y, transform.getSize().z));

               btVector3 localInertia = {0, 0, 0};
               shape->calculateLocalInertia(collision.mass, localInertia);

               btTransform shapeTransform;
               shapeTransform.setIdentity();
               shapeTransform.setOrigin(btVector3(transform.getPosition().x, transform.getPosition().y, transform.getPosition().z));

               glm::quat q(transform.getRotation());
               shapeTransform.setRotation(btQuaternion(q.x, q.y, q.z, q.w));

               auto* motionState = new btDefaultMotionState(shapeTransform);

               btRigidBody::btRigidBodyConstructionInfo rbInfo(collision.mass, motionState, shape, localInertia);

               auto* rigiBody = new btRigidBody(rbInfo);
               rigiBody->setUserIndex((int)entity.id);
               rigiBody->setCollisionFlags(rigiBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);

               dynamicsWorld->addRigidBody(rigiBody);
               collision.rigiBodies.push_back(rigiBody);
           }
        }
    }

} // namespace engine