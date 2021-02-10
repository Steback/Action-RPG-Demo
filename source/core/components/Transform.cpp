#include "Transform.hpp"


#include "glm/gtc/matrix_transform.hpp"


namespace core {


    Transform::Transform() = default;

    Transform::Transform(const glm::vec3 &position, const glm::vec3 &size, float velocity, float angle)
        : m_position(position), m_size(size), m_velocity(velocity), m_angle(angle) {

    }

    void Transform::update(float deltaTime) {
        m_position += m_velocity * deltaTime;

        m_angle += 10.0f * deltaTime;

        if (m_angle > 360) m_angle -= 360.0f;
    }

    glm::mat4 Transform::getModelMatrix() const {
        glm::mat4 model(1.0f);

        model = glm::translate(model, m_position);
        model = glm::scale(model, m_size);
        model = glm::rotate(model, glm::radians(m_angle), glm::vec3(0.0f, 1.0f, 0.0f));

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

    float Transform::getVelocity() const {
        return m_velocity;
    }

    void Transform::setVelocity(float velocity) {
        m_velocity = velocity;
    }

    float Transform::getAngle() const {
        return m_angle;
    }

    void Transform::setAngle(float angle) {
        m_angle = angle;
    }

}
