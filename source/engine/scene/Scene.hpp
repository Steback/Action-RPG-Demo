#ifndef PROTOTYPE_ACTION_RPG_SCENE_HPP
#define PROTOTYPE_ACTION_RPG_SCENE_HPP


#include <string>
#include <vector>
#include <mutex>

#include "entt/entt.hpp"
#include "nlohmann/json.hpp"

#include "../camera/Camera.hpp"
#include "../resources/ResourceManager.hpp"
#include "../Constants.hpp"
#include "../components/Transform.hpp"
#include "../components/ModelInterface.hpp"
#include "../renderer/GraphicsPipeline.hpp"
#include "../components/AnimationInterface.hpp"

using json = nlohmann::json;

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"


namespace engine {

    enum EntityType {
        OBJECT = 0,
        PLAYER = 1,
        ENEMY = 2,
        BUILDING = 3,
        CAMERA = 4
    };

    enum ComponentFlags {
        TRANSFORM = 1 << 0,
        MODEL = 1 << 1,
        ANIMATION = 1 << 2,
    };

    struct Entity {
        entt::entity enttID;
        uint32_t id;
        std::string name;
        uint32_t components{};
        uint32_t type;

        static void setLuaBindings(sol::table& table);
    };

    class Scene {
    public:
        Scene();

        ~Scene();

        void update(float deltaTime);

        void render(vk::CommandBuffer& cmdBuffer, const std::shared_ptr<GraphicsPipeline>& pipeAnimation);

        void cleanup();

        engine::Entity& addEntity(const std::string& name, uint32_t type);

        engine::Entity& getEntity(size_t ID);

        std::vector<engine::Entity>& getEntities();

        size_t getEntitiesCount();

        engine::Camera& getCamera();

        void loadScene(const std::string& uri, bool editorBuild = false, std::vector<std::string>* modelNames = nullptr,
                       std::unordered_map<uint32_t, std::string>* animationsName = nullptr);

        void saveScene(const std::string& uri, bool editorBuild = false, std::unordered_map<uint32_t, std::string>* animationsName = nullptr);

        entt::registry& registry();

        void setLuaBindings(sol::state& state);

        template<typename T>
        T& getComponent(uint32_t id) {
            return m_registry.get<T>(m_entities[id].enttID);
        }

    private:
        Transform& getTransform(uint32_t id);

        Camera& getCameraComponent(uint32_t id);

        ModelInterface& getModel(uint32_t id);

        AnimationInterface& getAnimation(uint32_t id);

    private:
        std::vector<engine::Entity> m_entities;
        engine::Camera m_camera{};
        entt::entity m_currentEntity{};
        entt::registry m_registry;
    };

}


#endif //PROTOTYPE_ACTION_RPG_SCENE_HPP
