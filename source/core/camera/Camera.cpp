#include "Camera.hpp"

#include "glm/gtc/matrix_transform.hpp"


namespace core {

    Camera::Camera() = default;


    Camera::~Camera() = default;

    glm::vec3 &Camera::getEye() {
        return m_eye;
    }

    glm::vec3 &Camera::getCenter() {
        return m_center;
    }

    glm::vec3 &Camera::getUp() {
        return m_up;
    }

    glm::mat4 Camera::getView() const {
        return glm::lookAt(m_eye, m_center, m_up);
    }

} // namespace core
