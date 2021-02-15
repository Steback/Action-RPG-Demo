#ifndef PROTOTYPE_ACTION_RPG_TRANSFORM_HPP
#define PROTOTYPE_ACTION_RPG_TRANSFORM_HPP


#include "glm/glm.hpp"
#include <glm/detail/type_quat.hpp>


namespace core {

    class Transform {
    public:
        Transform();

        Transform(const glm::mat4& model, const glm::vec3& position, const glm::vec3& size, float velocity, const glm::vec3& rotation);

        void update(float deltaTime);

        [[nodiscard]] glm::mat4 worldTransformMatrix() const;

        [[nodiscard]] glm::vec3 &getPosition();

        void setPosition(const glm::vec3 &position);

        [[nodiscard]] glm::vec3 &getSize();

        void setSize(const glm::vec3 &size);

        [[nodiscard]] float& getVelocity();

        void setVelocity(float velocity);

        [[nodiscard]] glm::vec3& getRotation();

        void setRotation(glm::vec3 rotation);

        glm::mat4& getTransform();

    private:
        glm::mat4 m_worldTransform{};
        glm::mat4 m_localTransform{};
        glm::vec3 m_position{};
        glm::vec3 m_size{};
        glm::vec3 m_rotation{};
        float m_velocity{};
    };

}


#endif //PROTOTYPE_ACTION_RPG_TRANSFORM_HPP
