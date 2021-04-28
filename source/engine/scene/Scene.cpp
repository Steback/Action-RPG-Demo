#include "Scene.hpp"

#include <fstream>
#include <iomanip>

#include "fmt/format.h"
#include "imgui.h"

#include "../Application.hpp"
#include "../renderer/CommandList.hpp"


namespace engine {

    void Entity::setLuaBindings(sol::table& table) {
        table.new_usertype<Entity>("Entity",
                                   "id", &Entity::id,
                                   "name", &Entity::name,
                                   "components", &Entity::components,
                                   "type", &Entity::type);
    }

    Scene::Scene() = default;

    Scene::~Scene() = default;

    void Scene::update(float deltaTime) {
        auto viewCamera = m_registry.view<Camera>();
        for (auto& entity : viewCamera) {
            Application::m_threadPool->submit([camera = &m_registry.get<Camera>(entity), transform = &m_registry.get<Transform>(entity)]{
                glm::vec2 angles = camera->getEulerAngles();
                camera->setDirection(angles.x, angles.y);
                transform->getPosition() = camera->getCenter() + (camera->getDirection() * camera->getDistance());
                camera->getEye() = transform->getPosition();
            });
        }

        for (auto& entity : m_entities) {
            if (entity.type != EntityType::CAMERA) {
                Application::m_threadPool->submit([transform = &m_registry.get<engine::Transform>(entity.enttID), deltaTime] {
                    transform->update(deltaTime);
                });
            }
        }
    }

    void Scene::render(vk::CommandBuffer& cmdBuffer, const std::shared_ptr<GraphicsPipeline>& pipeline) {
        auto view = m_registry.view<engine::Model>();

        for (auto& entity : view) {
            view.get<Model>(entity).render(cmdBuffer, pipeline->getLayout());;
        }
    }

    void Scene::cleanup() {
        for (auto& entity : m_entities) {
            m_registry.destroy(entity.enttID);
        }

        m_entities.clear();
    }

    engine::Entity& Scene::addEntity(const std::string &name, uint32_t type) {
        engine::Entity entity;
        entity.enttID = m_registry.create();
        entity.name = name;
        entity.id = m_entities.size();
        entity.components = 0;
        entity.type = type;

        m_entities.push_back(entity);

        return m_entities[entity.id];
    }

    engine::Entity &Scene::getEntity(size_t ID) {
        return m_entities[ID];
    }

    std::vector<engine::Entity> &Scene::getEntities() {
        return m_entities;
    }

    size_t Scene::getEntitiesCount() {
        return m_entities.size();
    }

    engine::Camera &Scene::getCamera() {
        return m_camera;
    }

    void Scene::loadScene(const std::string &uri, bool editorBuild, std::vector<std::string>* modelNames) {
        cleanup();

        json scene;
        std::ifstream file(uri);
        file >> scene;
        file.close();

        auto camera = scene["camera"];
        glm::vec3 target = {camera["target"]["x"].get<float>(), camera["target"]["y"].get<float>(), camera["target"]["z"].get<float>()};

        if (editorBuild) {
            auto& entity = addEntity("Camera", EntityType::OBJECT | EntityType::CAMERA);

            m_registry.emplace<engine::Model>(entity.enttID, engine::tools::hashString("cube"), entity.id);

            glm::vec3 direction;
            float yaw = glm::radians(camera["angles"]["yaw"].get<float>());
            float pitch = glm::radians(camera["angles"]["pitch"].get<float>());
            float distance = camera["distance"];

            direction.x = glm::cos(yaw) * glm::cos(pitch);
            direction.y = glm::sin(pitch);
            direction.z = glm::sin(yaw) * glm::cos(pitch);

            glm::vec3 pos = target + (direction * distance);
            auto speed = camera["speed"].get<float>();

            m_registry.emplace<engine::Transform>(entity.enttID, pos, DEFAULT_SIZE * 0.1f, speed, direction);
            m_registry.emplace<engine::Camera>(entity.enttID, glm::vec2(yaw, pitch), target, speed, camera["rotateSpeed"].get<float>(), distance);

            entity.components = ComponentFlags::MODEL | ComponentFlags::TRANSFORM;
        } else {
            m_camera = engine::Camera({camera["angles"]["yaw"].get<float>(), camera["angles"]["pitch"].get<float>()},
                                      target, camera["speed"].get<float>(), camera["rotateSpeed"].get<float>(),
                                      camera["distance"]);
        }

        for (auto& e : scene["entities"]) {
            auto& entity = addEntity(e["name"], e["type"]);

            if (!e["transform"].empty()) {
                auto& transform = e["transform"];

                m_registry.emplace<engine::Transform>(entity.enttID,
                                                      glm::vec3(transform["position"][0].get<float>(), transform["position"][1].get<float>(), transform["position"][2].get<float>()),
                                                      glm::vec3(transform["size"][0].get<float>(), transform["size"][1].get<float>(), transform["size"][2].get<float>()),
                                                      transform["speed"].get<float>(),
                                                      glm::vec3(transform["rotation"][0].get<float>(), transform["rotation"][1].get<float>(), transform["rotation"][2].get<float>()));
            }

            if (!e["model"].empty()) {
                auto& model = e["model"];

                m_registry.emplace<engine::Model>(entity.enttID,
                                                  engine::Application::m_resourceManager->createModel(model["name"].get<std::string>() + ".gltf", model["name"]),
                                                  entity.id);

                if (modelNames) modelNames->push_back(model["name"].get<std::string>());
            }

            entity.components = ComponentFlags::TRANSFORM | ComponentFlags::MODEL;
        }
    }

    void Scene::saveScene(const std::string &uri, bool editorBuild) {
        json scene;
        engine::Camera* camera;

        if (editorBuild) {
            camera = &m_registry.get<engine::Camera>(m_entities[0].enttID);
        } else {
            camera = &m_camera;
        }

        auto target = camera->getCenter();
        auto angles = glm::degrees(camera->getEulerAngles());

        scene["camera"] = {
                {"target", {
                    {"x", target.x},
                    {"y", target.y},
                    {"z", target.z}
                } },
                { "angles", {
                    {"yaw", angles.x},
                    {"pitch", angles.y}
                } },
                { "speed", camera->getSpeed(), },
                { "rotateSpeed", camera->getTurnSpeed() },
                { "distance", camera->getDistance() }
        };

        scene["entities"] = {};

        for (auto& entity : m_entities) {
            if (entity.type != EntityType::CAMERA) {
                size_t entitiesCount = scene["entities"].size();

                scene["entities"].push_back({
                    {"name", entity.name},
                    {"type", entity.type}
                });

                if (entity.components & engine::TRANSFORM) {
                    auto& transform = m_registry.get<engine::Transform>(entity.enttID);
                    auto& position = transform.getPosition();
                    auto& rotation = transform.getRotation();
                    auto& size = transform.getSize();

                    scene["entities"][entitiesCount]["transform"] = {
                        { "position", {
                            position.x,
                            position.y,
                            position.z
                        } },
                        { "rotation", {
                            rotation.x,
                            rotation.y,
                            rotation.z
                        } },
                        { "size", {
                            size.x,
                            size.y,
                            size.z
                        } },
                        {"speed", transform.getSpeed()}
                    };
                }

                if (entity.components & engine::MODEL) {
                    auto& model = m_registry.get<engine::Model>(entity.enttID);

                    scene["entities"][entitiesCount]["model"] = {
                            {"name", model.getName()}
                    };
                }
            }
        }

        std::ofstream file(uri.c_str());
        file << std::setw(4) << scene;
        file.close();
    }

    entt::registry &Scene::registry() {
        return m_registry;
    }

    void Scene::setLuaBindings(sol::state &state) {
        sol::table scene = state["scene"].get_or_create<sol::table>();
        sol::table components = state["components"].get_or_create<sol::table>();

        Transform::setLuaBindings(components);
        Model::setLuaBindings(components);

        components.new_enum("type",
                       "transform", ComponentFlags::TRANSFORM,
                       "model", ComponentFlags::MODEL);

        scene.new_enum("EntityType",
                       "player", EntityType::PLAYER,
                       "enemy", EntityType::ENEMY,
                       "building", EntityType::BUILDING,
                       "camera", EntityType::CAMERA);

        Camera::setLuaBindings(scene);
        Entity::setLuaBindings(scene);

        scene.set_function("getCamera", &Scene::getCamera, this);
        scene.set_function("getEntity", &Scene::getEntity, this);
        scene["entities"] = std::ref(m_entities);

        sol::table entityComponents = scene["components"].get_or_create<sol::table>();
        entityComponents.set_function("getTransform", &Scene::getTransform, this);
        entityComponents.set_function("getCamera", &Scene::getCameraComponent, this);
        entityComponents.set_function("getModel", &Scene::getModel, this);
    }

    Transform &Scene::getTransform(uint32_t id) {
        return getComponent<Transform>(id);
    }

    Camera &Scene::getCameraComponent(uint32_t id) {
        return getComponent<Camera>(id);
    }

    Model &Scene::getModel(uint32_t id) {
        return getComponent<Model>(id);
    }

} // namespace core
