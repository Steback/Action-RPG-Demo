#include "Movement.hpp"

#include "Transform.hpp"
#include "AnimationInterface.hpp"
#include "../Application.hpp"
#include "../MousePicking/MousePicking.hpp"


engine::Movement::Movement(Transform& transform) {
    moveTo = transform.getPosition();
}

void engine::Movement::update(float deltaTime, engine::Transform *transform, engine::AnimationInterface *animation) {
   if (isActive) {
       glm::vec3& position = transform->getPosition();
       glm::vec3& rotation = transform->getRotation();

       float offset = transform->getSpeed() * deltaTime;

       float distanceX = moveTo.x - position.x;
       float distanceZ = moveTo.z - position.z;

       isMoving = glm::sqrt(distanceX * distanceX + distanceZ * distanceZ) < offset;

       if (isMoving) {
           position.x = moveTo.x;
           position.z = moveTo.z;
       } else {
           animation->currentAnimation = Animation::walk;
           float radian = glm::atan(distanceZ, distanceX);
           position.x += cos(radian) * offset;
           position.z += sin(radian) * offset;
           rotation.y = -radian + (glm::pi<float>() * 0.5f);
       }
   }
}
