#include "Scene.hpp"

#include <fstream>
#include <iomanip>

#include "fmt/format.h"
#include "imgui.h"

#include "../Application.hpp"


namespace core {

    Scene::Scene() = default;

    Scene::~Scene() = default;

    void Scene::update(float deltaTime) {
        for (auto& entity : m_entities) {
            if (entity.flags & EntityFlags::CAMERA) {
                auto& camera = m_registry.get<core::Camera>(entity.enttID);
                auto& transform = m_registry.get<core::Transform>(entity.enttID);

                glm::vec2 angles = camera.getEulerAngles();

                camera.setDirection(angles.x, angles.y);

                transform.getPosition() = camera.getCenter() + (camera.getDirection() * camera.getDistance());
                camera.getEye() = transform.getPosition();
            } else {
                m_registry.get<core::Transform>(entity.enttID).update(deltaTime);
            }
        }
    }

    void Scene::render(vk::CommandBuffer& cmdBuffer, const vk::PipelineLayout& layout, const vk::DescriptorSet& set, MVP& mvp) {
        auto view = m_registry.view<core::Model>();

        for (auto& entity : view) {
            view.get<core::Model>(entity).render(cmdBuffer, layout, set, mvp);
        }
    }

    void Scene::cleanup() {
        for (auto& entity : m_entities) {
            m_registry.destroy(entity.enttID);
        }

        m_entities.clear();
    }

    core::Entity& Scene::addEntity(const std::string &name, uint32_t flags) {
        core::Entity entity;
        entity.enttID = m_registry.create();
        entity.name = name;
        entity.id = m_entities.size();
        entity.components = 0;
        entity.flags = flags;

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
        cleanup();

        json scene;
        std::ifstream file(uri);
        file >> scene;
        file.close();

        auto camera = scene["camera"];
        glm::vec3 target = {camera["target"]["x"].get<float>(), camera["target"]["y"].get<float>(), camera["target"]["z"].get<float>()};

        if (editorBuild) {
            auto& entity = addEntity("Camera", EntityFlags::OBJECT | EntityFlags::CAMERA);

            m_registry.emplace<core::Model>(entity.enttID, core::tools::hashString("cube"), entity.id);

            glm::vec3 direction;
            float yaw = glm::radians(camera["angles"]["yaw"].get<float>());
            float pitch = glm::radians(camera["angles"]["pitch"].get<float>());
            float distance = camera["distance"];

            direction.x = glm::cos(yaw) * glm::cos(pitch);
            direction.y = glm::sin(pitch);
            direction.z = glm::sin(yaw) * glm::cos(pitch);

            glm::vec3 pos = target + (direction * distance);
            auto speed = camera["speed"].get<float>();

            m_registry.emplace<core::Transform>(entity.enttID, pos, DEFAULT_SIZE * 0.1f, speed, direction);
            m_registry.emplace<core::Camera>(entity.enttID, glm::vec2(yaw, pitch), target, speed, camera["rotateSpeed"].get<float>(), distance);

            entity.components = ComponentFlags::MODEL | ComponentFlags::TRANSFORM;
        } else {
            m_camera = core::Camera({camera["angles"]["yaw"].get<float>(), camera["angles"]["pitch"].get<float>()},
                                    target, camera["speed"].get<float>(), camera["rotateSpeed"].get<float>(),
                                    camera["distance"]);
        }

        for (auto& e : scene["entities"]) {
            auto& entity = addEntity(e["name"], EntityFlags::OBJECT);

            if (!e["transform"].empty()) {
                auto& transform = e["transform"];

                m_registry.emplace<core::Transform>(entity.enttID,
                                                    glm::vec3(transform["position"][0].get<float>(), transform["position"][1].get<float>(), transform["position"][2].get<float>()),
                                                    glm::vec3(transform["size"][0].get<float>(), transform["size"][1].get<float>(), transform["size"][2].get<float>()),
                                                    transform["speed"].get<float>(),
                                                    glm::vec3(transform["rotation"][0].get<float>(), transform["rotation"][1].get<float>(), transform["rotation"][2].get<float>()));
            }

            if (!e["model"].empty()) {
                auto& model = e["model"];

                m_registry.emplace<core::Model>(entity.enttID,
                                                core::Application::m_resourceManager->createModel(model["name"].get<std::string>() + ".gltf", model["name"]),
                                                entity.id);
            }

            entity.components = ComponentFlags::TRANSFORM | ComponentFlags::MODEL;
        }
    }

    void Scene::saveScene(const std::string &uri, bool editorBuild) {
        json scene;
        core::Camera* camera;

        if (editorBuild) {
            camera = &m_registry.get<core::Camera>(m_entities[0].enttID);
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
            if (!(entity.flags & EntityFlags::CAMERA)) {
                size_t entitiesCount = scene["entities"].size();

                scene["entities"].push_back({
                    {"name", entity.name},
                });

                if (entity.components & core::TRANSFORM) {
                    auto& transform = m_registry.get<core::Transform>(entity.enttID);
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

                if (entity.components & core::MODEL) {
                    auto& model = m_registry.get<core::Model>(entity.enttID);

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

} // namespace core
