#include "Collision.hpp"

#include "btBulletDynamicsCommon.h"
#include "fmt/format.h"


engine::Collision::Collision(uint32_t owner) : owner(owner) {

}

engine::Collision::~Collision() = default;

void engine::Collision::update(engine::Transform *transform) {
    btTransform t = rigiBodies[0]->getWorldTransform();
    transform->getPosition() = {t.getOrigin().x(), t.getOrigin().y(), t.getOrigin().z()};
}
