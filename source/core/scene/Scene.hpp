#ifndef PROTOTYPE_ACTION_RPG_SCENE_HPP
#define PROTOTYPE_ACTION_RPG_SCENE_HPP


#include <string>
#include <vector>

#include "entt/entt.hpp"
#include "nlohmann/json.hpp"

#include "../camera/Camera.hpp"
#include "../resources/ResourceManager.hpp"
#include "../Constants.hpp"
#include "../components/Transform.hpp"
#include "../components/MeshModel.hpp"

using json = nlohmann::json;


namespace core {

    enum EntityFlags {
        OBJECT = 1 << 0,
        PLAYER = 1 << 1,
        ENEMY = 1 << 2,
        CAMERA = 1 << 3,
        BUILDING = 1 << 4
    };

    enum ComponentFlags {
        TRANSFORM = 1 << 0,
        MODEL = 1 << 1,
    };

    struct Entity {
        entt::entity enttID;
        uint32_t id;
        std::string name;
        uint32_t components;
        uint64_t flags;
    };

    class Scene {
    public:
        Scene();

        ~Scene();

        void update(float deltaTime);

        void render();

        void cleanup();

        core::Entity& addEntity(const std::string& name, uint32_t flags);

        core::Entity& getEntity(size_t ID);

        std::vector<core::Entity>& getEntities();

        size_t getEntitiesCount();

        core::Camera& getCamera();

        void loadScene(const std::string& uri, bool editorBuild = false);

        void saveScene(const std::string& uri, bool editorBuild = false);

        entt::registry& registry();

        template<typename T>
        T& getComponent(uint32_t id) {
            return m_registry.get<T>(m_entities[id].enttID);
        }

    private:
        void drawNode(const core::Model::Node& node, core::Model& model);

    private:
        std::vector<core::Entity> m_entities;
        core::Camera m_camera{};
        entt::entity m_currentEntity{};
        entt::registry m_registry;
    };

}


#endif //PROTOTYPE_ACTION_RPG_SCENE_HPP
