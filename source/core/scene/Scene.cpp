#include "Scene.hpp"

#include <fstream>

#include "GLFW/glfw3.h"
#include "fmt/format.h"
#include "imgui.h"

#include "../Application.hpp"
#include "../components/MeshModel.hpp"


namespace core {

    Scene::Scene() = default;

    Scene::~Scene() = default;

    void Scene::update(float deltaTime) {
        auto view = m_registry.view<core::Transform>();

        for (auto& entity : view) {
            auto& transform = view.get<core::Transform>(entity);
            transform.update(deltaTime);
        }
    }

    void Scene::render() {
        auto view = m_registry.view<core::MeshModel>();

        for (auto& entity : view) {
            auto& meshModel = view.get<core::MeshModel>(entity);
            core::Model& model = core::Application::resourceManager->getModel(meshModel.getModelID());
            m_currentEntity = entity;

            for (auto& node : model.getNodes()) {
                drawNode(node, model);
            }
        }
    }

    void Scene::cleanup() {

    }

    core::Entity& Scene::addEntity(const std::string &name, core::EntityType type) {
        core::Entity entity;
        entity.enttID = m_registry.create();
        entity.name = name;
        entity.id = m_entities.size();
        entity.type = type;
        entity.components = 0;

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

    void Scene::loadScene(const std::string &uri, bool editorBuild) {
        json scene;
        std::ifstream file(uri);
        file >> scene;
        file.close();

        auto camera = scene["camera"];
        glm::vec3 target = {camera["target"]["x"].get<float>(), camera["target"]["y"].get<float>(), camera["target"]["z"].get<float>()};

        if (editorBuild) {
            auto& entity = addEntity("Camera", core::CAMERA);

            m_registry.emplace<core::MeshModel>(entity.enttID, core::tools::hashString("cube"));

            glm::vec3 direction;

            float yaw = glm::radians(camera["angles"]["yaw"].get<float>());
            float pitch = glm::radians(camera["angles"]["pitch"].get<float>());

            direction.x = glm::cos(yaw) * glm::cos(pitch);
            direction.y = glm::sin(pitch);
            direction.z = glm::sin(yaw) * glm::cos(pitch);

            glm::vec3 pos = target + (direction * camera["distance"].get<float>());

            m_registry.emplace<core::Transform>(entity.enttID, pos, DEFAULT_SIZE * 0.1f, SPEED_ZERO, DEFAULT_ROTATION);

            entity.components = ComponentFlags::MODEL | ComponentFlags::TRANSFORM;
        } else {
            m_camera = core::Camera({camera["angles"]["yaw"].get<float>(), camera["angles"]["pitch"].get<float>()},
                                    {0.0f, 1.0f, 0.f},
                                    target,
                                    camera["speed"].get<float>(), camera["rotateSpeed"].get<float>(),
                                    camera["distance"], 45.0f, 0.01f, 100.f);
        }

        for (auto& e : scene["entities"]) {

        }
    }

    void Scene::saveScene(const std::string &uri) {

    }

    void Scene::drawNode(const Model::Node &node, core::Model& model) {
        if (node.mesh > 0) {
            auto& transform = m_registry.get<core::Transform>(m_currentEntity);

            core::Application::renderer->renderMesh(core::Application::resourceManager->getMesh(node.mesh), transform.worldTransformMatrix() * node.matrix);
        }

        for (auto& child : node.children) {
            drawNode(child, model);
        }
    }

    entt::registry &Scene::registry() {
        return m_registry;
    }

} // namespace core
