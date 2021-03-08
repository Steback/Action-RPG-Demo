#include "Scene.hpp"

#include <fstream>

#include "GLFW/glfw3.h"
#include "fmt/format.h"

#include "../components/Transform.hpp"
#include "../components/MeshModel.hpp"


namespace core {

    Scene::Scene() = default;

    Scene::~Scene() = default;

    void Scene::update(entt::registry& registry, float deltaTime) {

        auto view = registry.view<core::Transform>();

        for (auto& entity : view) {
            auto& transform = view.get<core::Transform>(entity);
            transform.update(deltaTime);
        }
    }

    void Scene::render() {

    }

    void Scene::cleanup() {

    }

    core::Entity& Scene::addEntity(const std::string &name, entt::entity enttID, core::EntityType type) {
        core::Entity entity;
        entity.enttID = enttID;
        entity.name = name;
        entity.id = m_entities.size();
        entity.type = type;

        m_entities.push_back(entity);

        return m_entities[entity.id];
    }

    core::Entity &Scene::getEntity(size_t ID) {
        return m_entities[ID];
    }

    std::vector<core::Entity> &Scene::getEntities() {
        return m_entities;
    }

    size_t Scene::getEntitiesCount() {
        return m_entities.size();
    }

    core::Camera &Scene::getCamera() {
        return m_camera;
    }

    void Scene::loadScene(const std::string &uri, core::ResourceManager* resourceManager, entt::registry* registry, bool editorBuild) {
        json scene;
        std::ifstream file(uri);
        file >> scene;
        file.close();

        auto camera = scene["camera"];

        if (editorBuild) {
            auto enttID = registry->create();
            auto entity = addEntity("Camera", enttID, core::CAMERA);

            registry->emplace<core::MeshModel>(enttID, 0);

            glm::vec3 direction;
            glm::vec3 target = {camera["target"]["x"].get<float>(), camera["target"]["y"].get<float>(),
                                camera["target"]["z"].get<float>()};
            float yaw = glm::radians(camera["angles"]["yaw"].get<float>());
            float pitch = glm::radians(camera["angles"]["pitch"].get<float>());

            direction.x = glm::cos(yaw) * glm::cos(pitch);
            direction.y = glm::sin(pitch);
            direction.z = glm::sin(yaw) * glm::cos(pitch);

            glm::vec3 pos = target + (direction * camera["distance"].get<float>());

            registry->emplace<core::Transform>(enttID, pos, DEFAULT_SIZE, SPEED_ZERO, DEFAULT_ROTATION);
        } else {
            glm::vec3 target = {camera["target"]["x"].get<float>(), camera["target"]["y"].get<float>(),
                                camera["target"]["z"].get<float>()};

            m_camera = core::Camera({camera["angles"]["yaw"].get<float>(), camera["angles"]["pitch"].get<float>()},
                                    {0.0f, 1.0f, 0.f},
                                    {camera["target"]["x"].get<float>(), camera["target"]["y"].get<float>(), camera["target"]["z"].get<float>()},
                                    camera["speed"].get<float>(), camera["rotateSpeed"].get<float>(),
                                    camera["distance"], 45.0f, 0.01f, 100.f);
        }

        for (auto& e : scene["entities"]) {

        }
    }

    void Scene::saveScene(const std::string &uri, core::ResourceManager* resourceManager, entt::registry* registry) {

    }

} // namespace core
