#include "Camera.hpp"

#include "glm/gtc/matrix_transform.hpp"


namespace core {

    Camera::Camera() = default;

    Camera::Camera(float yaw, float pitch, const glm::vec3& up, const glm::vec3& target, float velocity, float distance,
                   float fovy, float zNear, float zFar) : m_eulerAngles({yaw, pitch}), m_up(up), m_target(target), m_velocity(velocity),
                   m_distance(distance), m_fovy(fovy), m_zNear(zNear), m_zFar(zFar) {
        setDirection(yaw, pitch);
    }

    Camera::~Camera() = default;

    void Camera::move(float deltaTime, const glm::vec2& angle, MoveType type) {
        if (type == MoveType::ROTATION) {
            m_eulerAngles += angle * deltaTime * m_velocity;

            setDirection(m_eulerAngles.x, m_eulerAngles.y);

            m_direction = (m_target + glm::normalize(m_direction)) * m_distance;
        } else if (type == MoveType::TRANSLATE) {
            // TODO: Add center movement in z axis
            m_target += glm::vec3(angle.x, angle.y, 0.0f) * deltaTime * (m_velocity * 0.05f);
            m_direction += glm::vec3(angle.x, angle.y, 0.0f) * deltaTime * (m_velocity * 0.05f);
        }
    }

    void Camera::setDirection(float yaw, float pitch) {
        m_direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        m_direction.y = sin(glm::radians(pitch));
        m_direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    }

    void Camera::setDistance(float deltaTime, glm::vec2 &distance, bool& scrolling) {
        if (scrolling) {
            m_fovy += -distance.y * (m_velocity * 5) * deltaTime;

            scrolling = false;
        }
    }

    glm::vec3 &Camera::getEye() {
        return m_direction;
    }

    glm::vec3 &Camera::getCenter() {
        return m_target;
    }

    glm::vec3 &Camera::getUp() {
        return m_up;
    }

    glm::mat4 Camera::getView() const {
        return glm::lookAt(m_direction, m_target, m_up);
    }

    glm::mat4 Camera::getProjection(float aspect) const {
        glm::mat4 proj = glm::perspective(glm::radians(m_fovy), aspect, m_zNear, m_zFar);
        proj[1][1] *= -1;

        return proj;
    }

    glm::mat4 Camera::getProjectionFlipY(float aspect) const {
        return glm::perspective(glm::radians(m_fovy), aspect, m_zNear, m_zFar);
    }

    glm::vec2 &Camera::getEulerAngles() {
        return m_eulerAngles;
    }

    float &Camera::getFovy() {
        return m_fovy;
    }

    float &Camera::getNearPlane() {
        return m_zNear;
    }

    float &Camera::getFarPlane() {
        return m_zFar;
    }

} // namespace core
