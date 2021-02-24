#include "Editor.hpp"

#include "imgui.h"
#include "glm/gtc/type_ptr.hpp"
#include "fmt/format.h"

#include "renderer/UIImGui.hpp"
#include "Gizmos.hpp"

namespace editor {

    Editor::Editor() : core::Application("Editor", true), m_currentOperation(ImGuizmo::OPERATION::TRANSLATE) {

    }

    Editor::~Editor() = default;

    void Editor::init() {
        m_scene->getCamera().init(45.0f, 45.0f, {0.0f, 1.0f, 0.0f}, 20.0f, 45.0f, 0.01f, 100.0f);

        m_resourceManager->createTexture("plain.png");
        auto enttID = m_registry.create();
        auto entity = m_scene->addEntity("Viking Room", enttID);

        auto& model = m_registry.emplace<core::Model>(enttID, m_resourceManager->createModel("viking-room.gltf"));
        auto& transform = m_registry.emplace<core::Transform>(enttID, model.getMatrixModel(), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, glm::vec3(0.0f));
    }

    void Editor::update() {
        cameraMovement();

        m_scene->update(m_registry, m_deltaTime);
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
        auto view = m_registry.view<core::Model>();

        for (auto& entity : view) {
            auto& model = view.get<core::Model>(entity);
            model.clean();
        }
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
            }
        }
        ImGui::End();
    }

    void Editor::cameraControls() {
        ImGui::Begin("Camera Controls", &m_cameraControls);
        {
            ImGui::InputFloat3("Eye", glm::value_ptr(m_scene->getCamera().getEye()));
            ImGui::InputFloat3("Front", glm::value_ptr(m_scene->getCamera().getCenter()));
            ImGui::InputFloat3("Up", glm::value_ptr(m_scene->getCamera().getUp()));
            ImGui::Separator();
            ImGui::InputFloat3("Euler Angles", glm::value_ptr(m_scene->getCamera().getEulerAngles()));
            ImGui::Separator();
            ImGui::InputFloat("FOV", &m_scene->getCamera().getFovy());
        }
        ImGui::End();
    }

    void Editor::cameraMovement() {
        auto& camera = m_scene->getCamera();

        camera.setDistance(m_deltaTime, m_window->getScrollOffset(), m_window->isScrolling());

        if (m_window->mouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) && m_window->keyPressed(GLFW_KEY_LEFT_ALT)) {
            camera.move(m_deltaTime, m_window->getCursorPos(), core::Camera::MoveType::ROTATION);
        }

        if (m_window->mouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) && m_window->keyPressed(GLFW_KEY_LEFT_SHIFT)) {
            camera.move(m_deltaTime, m_window->getCursorPos(), core::Camera::MoveType::TRANSLATE);
        }

        m_proj = camera.getProjection(m_window->aspect());
        m_renderer->updateVP(camera.getView(), m_proj);
    }

    void Editor::drawGizmo() {
        ImGuizmo::BeginFrame();

        glm::mat4 projMatrix = m_scene->getCamera().getProjectionFlipY(m_window->aspect());

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