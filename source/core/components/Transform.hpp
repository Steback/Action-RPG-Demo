#ifndef PROTOTYPE_ACTION_RPG_TRANSFORM_HPP
#define PROTOTYPE_ACTION_RPG_TRANSFORM_HPP


#include "glm/glm.hpp"


namespace core {

    class Transform {
    public:
        Transform();

        Transform(const glm::vec3& position, const glm::vec3& size, float velocity, const glm::vec3& rotation);

        void update(float deltaTime);

        [[nodiscard]] glm::mat4 transformMatrix(glm::mat4 model) const;

        [[nodiscard]] glm::vec3 &getPosition();

        void setPosition(const glm::vec3 &position);

        [[nodiscard]] glm::vec3 &getSize();

        void setSize(const glm::vec3 &size);

        [[nodiscard]] float& getVelocity();

        void setVelocity(float velocity);

        [[nodiscard]] glm::vec3& getRotation();

        void setRotation(glm::vec3 angle);

    private:
        glm::vec3 m_position{};
        glm::vec3 m_size{};
        glm::vec3 m_rotation{};
        float m_velocity{};
    };

}


#endif //PROTOTYPE_ACTION_RPG_TRANSFORM_HPP
