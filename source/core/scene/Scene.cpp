#include "Scene.hpp"

#include "GLFW/glfw3.h"
#include "glm/gtc/matrix_transform.hpp"

#include "../components/Transform.hpp"


namespace core {

    Scene::Scene() {
        m_camera.getEye() = {1.0f, 1.0f, 1.0f};
        m_camera.getCenter() = glm::vec3(0.0f, 0.0f, 0.0f);
        m_camera.getUp() = glm::vec3(0.0f, 1.0f, 0.0f);
    }

    Scene::~Scene() = default;

    void Scene::update(entt::registry& registry) {
        auto now = static_cast<float>(glfwGetTime());
        deltaTime = now - lastTime;
        lastTime = now;

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

} // namespace core
