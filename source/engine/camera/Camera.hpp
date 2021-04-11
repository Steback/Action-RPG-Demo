#ifndef PROTOTYPE_ACTION_RPG_CAMERA_HPP
#define PROTOTYPE_ACTION_RPG_CAMERA_HPP


#include "glm/glm.hpp"

#include "../Constants.hpp"


namespace engine {

    class Camera {
    public:

        Camera();

        explicit Camera(const glm::vec2& angles, const glm::vec3& target, float speed, float rotateSpeed, float distance);

        ~Camera();

        void move(float deltaTime, const glm::vec3& offset);

        void rotate(float deltaTime, const glm::vec2& offset);

        void setDirection(float yaw, float pitch, bool radians = true);

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

        float& getSpeed();

        float& getTurnSpeed();

        float& getDistance();

        glm::vec3 getDirection() const ;

    private:
        glm::vec3 m_position{};
        glm::vec3 m_direction{};
        glm::vec3 m_target{};
        glm::vec3 m_up = YUP;
        float m_speed{};
        float m_rotateSpeed{};
        glm::vec2 m_eulerAngles{};
        float m_yFov = FOV;
        float m_zNear = Z_NEAR_PLANE;
        float m_zFar = Z_FAR_PLANE;
        float m_distance{};
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_CAMERA_HPP
