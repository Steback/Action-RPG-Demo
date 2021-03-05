#include "Editor.hpp"

#include "imgui.h"
#include "glm/gtc/type_ptr.hpp"
#include "fmt/format.h"

#include "components/MeshModel.hpp"
#include "renderer/UIImGui.hpp"
#include "Gizmos.hpp"

namespace editor {

    Editor::Editor() : core::Application("Editor", true), m_currentOperation(ImGuizmo::OPERATION::TRANSLATE) {

    }

    Editor::~Editor() = default;

    void Editor::init() {
        m_scene->getCamera() = core::Camera({45.0f, 45.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 0.5f, 10.0f, 1.0f, 45.0f,
                                            0.01f, 100.0f);

        m_resourceManager->createTexture("plain.png", "plain");
        auto enttID = m_registry.create();
        auto entity = m_scene->addEntity("Hero", enttID);

        uint meshNodeID;
        m_resourceManager->createModel("hero.gltf", "hero", meshNodeID);

        auto& model = m_registry.emplace<core::MeshModel>(enttID, "hero", meshNodeID);
        auto& transform = m_registry.emplace<core::Transform>(enttID, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.1f, 0.1f, 0.1f), 0.0f, glm::vec3(0.0f));
    }

    void Editor::update() {
        cameraMovement();
    }

    void Editor::draw() {
        core::UIImGui::newFrame();

        ImGui::ShowDemoWindow();

        menuBar();
        entitiesPanel();

        if (m_cameraControls) cameraControls();

        drawGizmo();

        core::UIImGui::render();
    }

    void Editor::cleanup() {

    }

    void Editor::menuBar() {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                ImGui::MenuItem("Save", nullptr, nullptr);
            }

            if (ImGui::BeginMenu("Tools")) {
                if (ImGui::MenuItem("Camera Controls", nullptr)) m_cameraControls = !m_cameraControls;
            }
        }
    }

    void Editor::entitiesPanel() {
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        ImGui::SetNextWindowPos({0.0f, 22});
        ImGui::SetNextWindowSize({350.0f, ((io.DisplaySize.y * 0.5f) - 22)});
        ImGui::Begin("Entities", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
        {
            for (size_t i = 0; i < m_scene->getEntitiesCount(); ++i) {
                auto entity = m_scene->getEntity(i);
                char buf[32];

                sprintf(buf, "%s", entity.name.c_str());

                if (ImGui::Selectable(buf, entitySelected == i)) entitySelected = i;
            }

        }
        ImGui::End();

        ImGui::SetNextWindowPos({0.0f, io.DisplaySize.y * 0.5f});
        ImGui::SetNextWindowSize({350.0f, io.DisplaySize.y * 0.5f});
        ImGui::Begin("Entity Properties", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
        {
            if (entitySelected != -1) {
                auto& entity = m_scene->getEntity(entitySelected);

                ImGui::Text("Entity ID: %i", entity.id);

                char* entityName = entity.name.data();
                ImGui::InputText("Entity name", entityName, 30);

                entity.name = entityName;

                if (ImGui::CollapsingHeader("Transform")) {
                    auto& transform = m_registry.get<core::Transform>(entity.enttID);

                    ImGui::InputFloat3("Position", glm::value_ptr(transform.getPosition()));
                    ImGui::InputFloat3("Size", glm::value_ptr(transform.getSize()));
                    ImGui::InputFloat3("Rotation", glm::value_ptr(transform.getRotation()));
                    ImGui::InputFloat("Velocity", &transform.getVelocity());
                }

                if (ImGui::CollapsingHeader("Model")) {
                    auto meshModel = m_registry.get<core::MeshModel>(entity.enttID);

                    ImGui::Text("Name: %s", meshModel.getModelName().c_str());
                }
            }
        }
        ImGui::End();
    }

    void Editor::cameraControls() {
        ImGui::SetNextWindowSize({-1, -1});
        ImGui::Begin("Camera Controls", &m_cameraControls);
        {
            ImGui::InputFloat3("Eye", glm::value_ptr(m_scene->getCamera().getEye()));
            ImGui::InputFloat3("Front", glm::value_ptr(m_scene->getCamera().getCenter()));
            ImGui::InputFloat3("Up", glm::value_ptr(m_scene->getCamera().getUp()));
            ImGui::Separator();
            ImGui::InputFloat2("Euler Angles", glm::value_ptr(m_scene->getCamera().getEulerAngles()));
            ImGui::Separator();
            ImGui::InputFloat("FOV", &m_scene->getCamera().getFovy());
            ImGui::Separator();
            ImGui::InputFloat("Velocity", &m_scene->getCamera().getVelocity());
            ImGui::InputFloat("Turn Velocity", &m_scene->getCamera().getTurnVelocity());
            ImGui::Separator();
            ImGui::InputFloat("Distances", &m_scene->getCamera().getDistance());
        }
        ImGui::End();
    }

    void Editor::cameraMovement() {
        auto& camera = m_scene->getCamera();

        camera.setZoom(m_deltaTime, m_window->getScrollOffset(), m_window->isScrolling());

        if (m_window->mouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) && m_window->keyPressed(GLFW_KEY_LEFT_ALT)) {
            camera.rotate(m_deltaTime, m_window->getCursorPos());
        }

        if (m_window->mouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) && m_window->keyPressed(GLFW_KEY_LEFT_SHIFT)) {
            // Work "fine" for now, I hope in future learn how to do this correctly
            glm::vec3 direction;
            glm::vec2 cursorChange = m_window->getCursorPos();
            glm::vec2 camAngles = m_scene->getCamera().getEulerAngles();

            direction.x = cursorChange.x * glm::sin(camAngles.x);
            direction.y = cursorChange.y;
            direction.z = cursorChange.x * -glm::cos(camAngles.x);

            camera.move(m_deltaTime, direction);
        }

        m_proj = camera.getProjection(m_window->aspect());
        m_renderer->updateVP(camera.getView(), m_proj);
    }

    void Editor::drawGizmo() {
        ImGuizmo::BeginFrame();

        glm::mat4 projMatrix = m_scene->getCamera().getProjection(m_window->aspect(), false);

        if (entitySelected != -1) {
            if (m_gizmoDraw) ImGuizmo::Enable(m_gizmoDraw);

            m_gizmoDraw = false;

            if (m_window->keyPressed(GLFW_KEY_T)) {
                m_currentOperation = ImGuizmo::OPERATION::TRANSLATE;
                m_window->setKeyValue(GLFW_KEY_T, false);
            } else if (m_window->keyPressed(GLFW_KEY_R)) {
                m_currentOperation = ImGuizmo::OPERATION::ROTATE;
                m_window->setKeyValue(GLFW_KEY_R, false);
            } else if (m_window->keyPressed(GLFW_KEY_S)) {
                m_currentOperation = ImGuizmo::OPERATION::SCALE;
                m_window->setKeyValue(GLFW_KEY_S, false);
            }

            auto& transform = m_registry.get<core::Transform>(m_scene->getEntity(entitySelected).enttID);
            editor::gizmo::transform(transform, m_currentOperation, m_scene->getCamera().getView(), projMatrix);
        }
    }

} // namespace editor