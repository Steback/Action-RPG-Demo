#include "Transform.hpp"


#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"


namespace engine {


    Transform::Transform() = default;

    Transform::Transform(const glm::vec3 &position, const glm::vec3 &size, float velocity, const glm::vec3& rotation)
        : m_position(position), m_size(size), m_speed(velocity), m_rotation(rotation) {

    }

    void Transform::update(float deltaTime) {

    }

    glm::mat4 Transform::worldTransformMatrix() const {
        glm::mat4 model(1.0f);

        model = glm::translate(model, m_position);
        model = glm::scale(model, m_size) ;
        model *= glm::mat4(glm::quat(m_rotation));

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

    float& Transform::getSpeed() {
        return m_speed;
    }

    void Transform::setSpeed(float speed) {
        m_speed = speed;
    }

    glm::vec3& Transform::getRotation() {
        return m_rotation;
    }

    void Transform::setRotation(glm::vec3 rotation) {
        m_rotation = rotation;
    }

    float *Transform::getSpeedPtr(bool ptr) {
        return &m_speed;
    }

    void Transform::setLuaBindings(sol::table &table) {
        table.new_usertype<Transform>("Transform",
                                      sol::call_constructor, sol::constructors<Transform(), Transform(const glm::vec3&, const glm::vec3&, float, const glm::vec3&)>(),
                                      "getPosition", &Transform::getPosition,
                                      "setPosition", &Transform::setPosition,
                                      "getRotation", &Transform::getRotation,
                                      "setRotation", &Transform::setRotation,
                                      "getScale", &Transform::getSize,
                                      "setScale", &Transform::setSize,
                                      "getVelocity", sol::overload(&Transform::getSpeed, &Transform::getSpeedPtr),
                                      "setVelocity", &Transform::setSpeed);
    }

}
