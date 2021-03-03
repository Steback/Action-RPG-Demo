#include "Transform.hpp"


#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"


namespace core {


    Transform::Transform() = default;

    Transform::Transform(const glm::vec3 &position, const glm::vec3 &size, float velocity, const glm::vec3& rotation)
        : m_position(position), m_size(size), m_velocity(velocity), m_rotation(rotation), m_worldTransform(1.0f) {

    }

    void Transform::update(float deltaTime) {

    }

    glm::mat4 Transform::worldTransformMatrix() const {
        glm::mat4 model(1.0f);

        // I'm noob, for this reason I not understand why I need to swap z-y values, and flip y
        model = glm::translate(model, glm::vec3(m_position.x, m_position.z, -m_position.y));
        model = glm::scale(model, glm::vec3(m_size.x, m_size.z, m_size.y));
        model *= glm::mat4(glm::quat(glm::vec3(m_rotation.x, m_rotation.z, -m_rotation.y)));

        return model;
    }

    glm::vec3 &Transform::getPosition()  {
        return m_position;
    }

    void Transform::setPosition(const glm::vec3 &position) {
        m_position = position;
    }

    glm::vec3 &Transform::getSize()  {
        return m_size;
    }

    void Transform::setSize(const glm::vec3 &size) {
        m_size = size;
    }

    float& Transform::getVelocity() {
        return m_velocity;
    }

    void Transform::setVelocity(float velocity) {
        m_velocity = velocity;
    }

    glm::vec3& Transform::getRotation() {
        return m_rotation;
    }

    void Transform::setRotation(glm::vec3 rotation) {
        m_rotation = rotation;
    }

    glm::mat4 &Transform::getTransform() {
        return m_worldTransform;
    }

}
