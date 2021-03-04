#ifndef PROTOTYPE_ACTION_RPG_CAMERA_HPP
#define PROTOTYPE_ACTION_RPG_CAMERA_HPP


#include "glm/glm.hpp"


namespace core {

    class Camera {
    public:

        enum MoveType {
            ROTATION,
            TRANSLATE
        };

        Camera();

        explicit Camera(float yaw, float pitch, const glm::vec3& up, const glm::vec3& target, float velocity, float distance,
                        float yFov, float zNear, float zFar);

        ~Camera();

        void move(float deltaTime, const glm::vec2& angle, MoveType type);

        void setDirection(float yaw, float pitch);

        void setZoom(float deltaTime, glm::vec2& offset, bool& scrolling);

        glm::vec3& getEye();

        glm::vec3& getCenter();

        glm::vec3& getUp();

        [[nodiscard]] glm::mat4 getView() const;

        [[nodiscard]] glm::mat4 getProjection(float aspect, bool flipY = true) const;

        glm::vec2& getEulerAngles();

        float& getFovy();

        float& getNearPlane();

        float& getFarPlane();

    private:
        glm::vec3 m_direction{};
        glm::vec3 m_target{};
        glm::vec3 m_up{};
        float m_velocity{};
        glm::vec2 m_eulerAngles{};
        float m_yFov{};
        float m_zNear{};
        float m_zFar{};
        float m_distance{};
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_CAMERA_HPP
