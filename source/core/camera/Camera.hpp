#ifndef PROTOTYPE_ACTION_RPG_CAMERA_HPP
#define PROTOTYPE_ACTION_RPG_CAMERA_HPP


#include "glm/glm.hpp"


namespace core {

    class Camera {
    public:
        Camera();

        ~Camera();

        glm::vec3& getEye();

        glm::vec3& getCenter();

        glm::vec3& getUp();

        [[nodiscard]] glm::mat4 getView() const;

        glm::mat4 getProjection(float fovy, float aspect, float zNear, float zFar);

        glm::mat4 getProjectionFlipY(float fovy, float aspect, float zNear, float zFar);

    private:
        glm::vec3 m_eye{};
        glm::vec3 m_center{};
        glm::vec3 m_up{};
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_CAMERA_HPP
