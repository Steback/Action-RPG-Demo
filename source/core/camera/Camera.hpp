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

        ~Camera();

        void init(float yaw, float pitch, const glm::vec3& up, float velocity, float fovy, float zNear, float zFar);

        void move(float deltaTime, const glm::vec2& angle, MoveType type);

        void setDirection(float yaw, float pitch);

        void setDistance(float deltaTime, glm::vec2& offset, bool& scrolling);

        glm::vec3& getEye();

        glm::vec3& getCenter();

        glm::vec3& getUp();

        [[nodiscard]] glm::mat4 getView() const;

        [[nodiscard]] glm::mat4 getProjection(float aspect) const;

        [[nodiscard]] glm::mat4 getProjectionFlipY(float aspect) const;

        glm::vec2& getEulerAngles();

        float& getFovy();

        float& getNearPlane();

        float& getFarPlane();

    private:
        glm::vec3 m_direction{};
        glm::vec3 m_front{};
        glm::vec3 m_up{};
        float m_velocity{};
        glm::vec2 m_eulerAngles{};
        float m_fovy{};
        float m_zNear{};
        float m_zFar{};
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_CAMERA_HPP
