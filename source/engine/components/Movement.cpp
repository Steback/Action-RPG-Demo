#include "Movement.hpp"

#include "Transform.hpp"
#include "AnimationInterface.hpp"
#include "../Application.hpp"


engine::Movement::Movement() = default;

void engine::Movement::update(float deltaTime, engine::Transform *transform, engine::AnimationInterface *animation) {
    glm::vec3& position = transform->getPosition();
    glm::vec3& rotation = transform->getRotation();

    if (Application::keys[Key::UP]) {
        animation->currentAnimation = Animation::walk;
        position.z -= transform->getSpeed() * deltaTime;
        rotation.y = glm::radians(180.0f);
        isMoving = true;
    } else if (Application::keys[Key::DOWN]) {
        animation->currentAnimation = Animation::walk;
        position.z += transform->getSpeed() * deltaTime;
        rotation.y = glm::radians(0.0f);
        isMoving = true;
    } else if (Application::keys[Key::LEFT]) {
        animation->currentAnimation = Animation::walk;
        position.x -= transform->getSpeed() * deltaTime;
        rotation.y = glm::radians(270.0f);
        isMoving = true;
    } else if (Application::keys[Key::RIGHT]) {
        animation->currentAnimation = Animation::walk;
        position.x += transform->getSpeed() * deltaTime;
        rotation.y = glm::radians(90.0f);
        isMoving = true;
    } else {
        animation->currentAnimation = Animation::idle;
        isMoving = false;
    }
}
