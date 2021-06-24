#include "Collision.hpp"

#include "btBulletDynamicsCommon.h"
#include "fmt/format.h"


engine::Collision::Collision(uint32_t owner, float mass, const glm::vec3& halfSize)
        : owner(owner), mass(mass), halfSize({halfSize.x, halfSize.y, halfSize.z}) {

}

engine::Collision::~Collision() = default;

void engine::Collision::update(engine::Transform *worldTransform) {
    transform = rigiBodies[0]->getWorldTransform();
    glm::vec3& position = worldTransform->getPosition();
    position.y = transform.getOrigin().y();
    transform.setOrigin({position.x, position.y, position.z});
    transform.setRotation({worldTransform->getRotation().x, worldTransform->getRotation().y, worldTransform->getRotation().z});
    rigiBodies[0]->setWorldTransform(transform);
}
