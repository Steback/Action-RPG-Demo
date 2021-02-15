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

    glm::mat4 Camera::getProjection(float fovy, float aspect, float zNear, float zFar) {
        glm::mat4 proj = glm::perspective(glm::radians(fovy), aspect, zNear, zFar);
        proj[1][1] *= -1;

        return proj;
    }

    glm::mat4 Camera::getProjectionFlipY(float fovy, float aspect, float zNear, float zFar) {
        return glm::perspective(glm::radians(fovy), aspect, zNear, zFar);
    }

} // namespace core
