#include "Camera.hpp"

#include "glm/gtc/matrix_transform.hpp"



namespace core {

    Camera::Camera() = default;

    Camera::Camera(const glm::vec2& angles, const glm::vec3& target, float speed, float rotateSpeed, float distance) :
                    m_eulerAngles(angles), m_up(YUP), m_target(target), m_speed(speed), m_rotateSpeed(rotateSpeed),
                    m_distance(distance) {
        setDirection(m_eulerAngles.x, m_eulerAngles.y, false);
        m_position = m_target + (m_direction * m_distance);
    }

    Camera::~Camera() = default;

    void Camera::move(float deltaTime, const glm::vec3& offset) {
        m_target += offset * deltaTime * m_speed;
        m_position += offset * deltaTime * m_speed;
    }

    void Camera::rotate(float deltaTime, const glm::vec2& offset) {
        m_eulerAngles += offset * deltaTime * m_rotateSpeed;

        setDirection(m_eulerAngles.x, m_eulerAngles.y, false);

        m_position = m_target + (m_direction * m_distance);
    }

    void Camera::setDirection(float yaw, float pitch, bool radians) {
        if (radians) {
            m_direction.x = glm::cos(yaw) * glm::cos(pitch);
            m_direction.y = glm::sin(pitch);
            m_direction.z = glm::sin(yaw) * glm::cos(pitch);
        } else {
            m_direction.x = glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
            m_direction.y = glm::sin(glm::radians(pitch));
            m_direction.z = glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
        }
    }

    void Camera::setZoom(float deltaTime, glm::vec2 &offset, bool& scrolling) {
        if (scrolling) {
            m_yFov -= offset.y * (m_speed * 100) * deltaTime;

            scrolling = false;
        }
    }

    glm::vec3 &Camera::getEye() {
        return m_position;
    }

    glm::vec3 &Camera::getCenter() {
        return m_target;
    }

    glm::vec3 &Camera::getUp() {
        return m_up;
    }

    glm::mat4 Camera::getView() const {
        return glm::lookAt(m_position, m_target, m_up);
    }

    glm::mat4 Camera::getProjection(float aspect, bool flipY) const {
        glm::mat4 proj = glm::perspective(glm::radians(m_yFov), aspect, m_zNear, m_zFar);

        if (flipY) proj[1][1] *= -1;

        return proj;
    }

    glm::vec2 &Camera::getEulerAngles() {
        return m_eulerAngles;
    }

    float &Camera::getFovy() {
        return m_yFov;
    }

    float &Camera::getNearPlane() {
        return m_zNear;
    }

    float &Camera::getFarPlane() {
        return m_zFar;
    }

    float &Camera::getSpeed() {
        return m_speed;
    }

    float &Camera::getTurnSpeed() {
        return m_rotateSpeed;
    }

    float &Camera::getDistance() {
        return m_distance;
    }

    glm::vec3 Camera::getDirection() const {
        return m_direction;
    }

} // namespace core
