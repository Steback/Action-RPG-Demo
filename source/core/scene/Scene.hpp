#ifndef PROTOTYPE_ACTION_RPG_SCENE_HPP
#define PROTOTYPE_ACTION_RPG_SCENE_HPP


#include <string>
#include <vector>

#include "entt/entt.hpp"

#include "../camera/Camera.hpp"


namespace core {

    struct Entity {
        entt::entity enttID;
        uint32_t id;
        std::string name;
    };

    class Scene {
    public:
        Scene();

        ~Scene();

        void update(entt::registry& registry);

        void render();

        void cleanup();

        core::Entity& addEntity(const std::string& name, entt::entity enttID);

        core::Entity& getEntity(size_t ID);

        std::vector<core::Entity>& getEntities();

        size_t getEntitiesCount();

        core::Camera& getCamera();

    private:
        std::vector<core::Entity> m_entities;
        float deltaTime{};
        float lastTime{};
        core::Camera m_camera{};
    };

}


#endif //PROTOTYPE_ACTION_RPG_SCENE_HPP
