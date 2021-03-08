#ifndef PROTOTYPE_ACTION_RPG_SCENE_HPP
#define PROTOTYPE_ACTION_RPG_SCENE_HPP


#include <string>
#include <vector>

#include "entt/entt.hpp"
#include "nlohmann/json.hpp"

#include "../camera/Camera.hpp"
#include "../resources/ResourceManager.hpp"
#include "../Constants.hpp"

using json = nlohmann::json;


namespace core {

    enum EntityType {
        CAMERA = 0,
        PLAYER = 1
    };

    struct Entity {
        entt::entity enttID;
        uint32_t id;
        std::string name;
        EntityType type;
    };

    class Scene {
    public:
        Scene();

        ~Scene();

        void update(entt::registry& registry, float deltaTime);

        void render();

        void cleanup();

        core::Entity& addEntity(const std::string& name, entt::entity enttID, core::EntityType type);

        core::Entity& getEntity(size_t ID);

        std::vector<core::Entity>& getEntities();

        size_t getEntitiesCount();

        core::Camera& getCamera();

        void loadScene(const std::string& uri, core::ResourceManager* resourceManager, entt::registry* registry, bool editorBuild = false);

        void saveScene(const std::string& uri, core::ResourceManager* resourceManager, entt::registry* registry);

    private:
        std::vector<core::Entity> m_entities;
        core::Camera m_camera{};
    };

}


#endif //PROTOTYPE_ACTION_RPG_SCENE_HPP
