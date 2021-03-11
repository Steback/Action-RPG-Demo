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
        m_resourceManager->createTexture("plain.png", "plain");
        m_resourceManager->createModel("cube.gltf", "cube");
        m_modelsNames.emplace_back("cube");

        m_scene->loadScene("../data/basicScene.json", m_resourceManager, &m_registry, true);

        m_scene->getCamera() = core::Camera({45.0f, 45.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 0.5f, 10.0f, 3.0f, 45.0f,
                                            0.01f, 100.0f);
    }

    void Editor::update() {
        cameraMovement();

        m_renderer->updateVP(m_scene->getCamera().getView(), m_scene->getCamera().getProjection(m_window->aspect()));
    }

    void Editor::draw() {
        core::UIImGui::newFrame();

        if (m_imguiDemo) ImGui::ShowDemoWindow(&m_imguiDemo);

        menuBar();
        entitiesPanel();

        if (m_cameraControls) cameraControls();

        if (m_addEntity) addEntity();

        if (m_addModel) addModel();

        drawGizmo();

        core::UIImGui::render();
    }

    void Editor::cleanup() {

    }

    void Editor::menuBar() {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                ImGui::MenuItem("New", nullptr, nullptr);
                ImGui::MenuItem("Save", nullptr, nullptr);
                ImGui::MenuItem("Open", nullptr, nullptr);

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Entity")) {
                if (ImGui::MenuItem("Add Entity")) m_addEntity = !m_addEntity;

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Assets")) {
                if (ImGui::MenuItem("Add Model")) m_addModel = !m_addModel;

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Tools")) {
                if (ImGui::MenuItem("Camera Controls", nullptr)) m_cameraControls = !m_cameraControls;
                if (ImGui::MenuItem("ImGui Demo", nullptr)) m_imguiDemo = !m_imguiDemo;

                ImGui::EndMenu();
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

                ImGui::InputText("Entity name", entity.name.data(), 30);

                std::vector<const char*> entityTypes = {"CAMERA", "PLAYER"};
                ImGui::Combo("Type", reinterpret_cast<int*>(&entity.type), entityTypes.data(), entityTypes.size());

                if (ImGui::CollapsingHeader("Transform")) {
                    auto& transform = m_registry.get<core::Transform>(entity.enttID);

                    ImGui::InputFloat3("Position", glm::value_ptr(transform.getPosition()));
                    ImGui::InputFloat3("Size", glm::value_ptr(transform.getSize()));
                    ImGui::InputFloat3("Rotation", glm::value_ptr(transform.getRotation()));
                    ImGui::InputFloat("Velocity", &transform.getSpeed());
                }

                if (ImGui::CollapsingHeader("Model")) {
                    auto& meshModel = m_registry.get<core::MeshModel>(entity.enttID);

                    ImGui::Text("Model ID: %lu", meshModel.getModelID());

                    static int currentModel = 0;
                    const char* currentModelName = m_modelsNames[currentModel].c_str();

                    if (ImGui::BeginCombo("Name", currentModelName)) {
                        for (int i = 0; i < m_modelsNames.size(); ++i) {
                            const bool is_selected = (currentModel == i);

                            if (ImGui::Selectable(m_modelsNames[i].c_str(), is_selected)) {
                                currentModel = i;
                                meshModel.setModelID(core::tools::hashString(m_modelsNames[currentModel]));
                            }

                            if (is_selected) ImGui::SetItemDefaultFocus();
                        }

                        ImGui::EndCombo();
                    }
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

        if (!m_addModel)
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
    }

    void Editor::drawGizmo() {
        ImGuizmo::BeginFrame();

        glm::mat4 projMatrix = m_scene->getCamera().getProjection(m_window->aspect(), false);

        if (entitySelected != -1) {
            if (m_gizmoDraw) ImGuizmo::Enable(m_gizmoDraw);

            m_gizmoDraw = false;

            if (m_window->keyPressed(GLFW_KEY_T) && !m_addEntity) {
                m_currentOperation = ImGuizmo::OPERATION::TRANSLATE;
                m_window->setKeyValue(GLFW_KEY_T, false);
            } else if (m_window->keyPressed(GLFW_KEY_R) && !m_addEntity) {
                m_currentOperation = ImGuizmo::OPERATION::ROTATE;
                m_window->setKeyValue(GLFW_KEY_R, false);
            } else if (m_window->keyPressed(GLFW_KEY_S) && !m_addEntity) {
                m_currentOperation = ImGuizmo::OPERATION::SCALE;
                m_window->setKeyValue(GLFW_KEY_S, false);
            }

            auto& transform = m_registry.get<core::Transform>(m_scene->getEntity(entitySelected).enttID);
            editor::gizmo::transform(transform, m_currentOperation, m_scene->getCamera().getView(), projMatrix);
        }
    }

    void Editor::addEntity() {
        auto enttID = m_registry.create();
        auto entity = m_scene->addEntity("Object", enttID, core::PLAYER);

        m_registry.emplace<core::MeshModel>(enttID, core::tools::hashString("cube"));
        m_registry.emplace<core::Transform>(enttID, m_scene->getCamera().getCenter(), DEFAULT_SIZE, SPEED_ZERO, DEFAULT_ROTATION);

        m_addEntity = !m_addEntity;
    }

    void Editor::addModel() {
        if (m_addModel)
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".gltf", ".");

        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
                int idx = filePathName.rfind('/');
                std::string fileName = filePathName.substr(idx + 1, filePathName.size());

                idx = fileName.rfind('.');
                std::string modelName = fileName.substr(0, idx);

                m_resourceManager->createModel(fileName, modelName);
                m_modelsNames.emplace_back(modelName);
            }

            ImGuiFileDialog::Instance()->Close();
            m_addModel = !m_addModel;
        }
    }

} // namespace editor