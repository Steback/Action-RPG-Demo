#ifndef PROTOTYPE_ACTION_RPG_TRANSFORM_HPP
#define PROTOTYPE_ACTION_RPG_TRANSFORM_HPP


#include "glm/glm.hpp"
#include <glm/detail/type_quat.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"


namespace engine {

    class Transform {
    public:
        Transform();

        Transform(const glm::vec3& position, const glm::vec3& size, float velocity, const glm::vec3& rotation);

        void update(float deltaTime);

        [[nodiscard]] glm::mat4 worldTransformMatrix() const;

        [[nodiscard]] glm::vec3 &getPosition();

        void setPosition(const glm::vec3 &position);

        [[nodiscard]] glm::vec3 &getSize();

        void setSize(const glm::vec3 &size);

        [[nodiscard]] float& getSpeed();

        void setSpeed(float speed);

        [[nodiscard]] glm::vec3& getRotation();

        void setRotation(glm::vec3 rotation);

        static void setLuaBindings(sol::table& table);

    private:
        float* getSpeedPtr(bool ptr);

    private:
        glm::vec3 m_position{};
        glm::vec3 m_size{};
        glm::vec3 m_rotation{};
        float m_speed{};
    };

}


#endif //PROTOTYPE_ACTION_RPG_TRANSFORM_HPP
