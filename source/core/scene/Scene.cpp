#include "Scene.hpp"

#include "GLFW/glfw3.h"
#include "glm/gtc/matrix_transform.hpp"

#include "../components/Transform.hpp"


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

    core::Entity& Scene::addEntity(const std::string &name, entt::entity enttID) {
        core::Entity entity;
        entity.enttID = enttID;
        entity.name = name;
        entity.id = m_entities.size();

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

    void Scene::loadScene(const std::string &uri) {

    }

    void Scene::saveScene(const std::string &uri) {

    }

} // namespace core
