#include "Editor.hpp"

#include "imgui.h"
#include "glm/gtc/type_ptr.hpp"
#include "fmt/format.h"
#include <glm/gtx/matrix_decompose.inl>

#include "components/MeshModel.hpp"
#include "renderer/UIImGui.hpp"
#include "Gizmos.hpp"


namespace editor {

    Editor::Editor() : core::Application("Editor", true), m_currentOperation(ImGuizmo::OPERATION::TRANSLATE) {

    }

    Editor::~Editor() = default;

    void Editor::init() {
        resourceManager->createModel("cube.gltf", "cube");
        m_modelsNames.emplace_back("cube");

        m_scene->loadScene("../data/basicScene.json", true);

        for (auto& entity : m_scene->getEntities()) {
            m_entitiesInfo.push_back({entity.id, entity.name, 0});
        }

        m_scene->getCamera() = core::Camera({45.0f, 45.0f}, {0.0f, 0.0f, 0.0f}, 0.5f, 10.0f, 10.0f);
    }

    void Editor::update() {
        cameraMovement();

        renderer->updateVP(m_scene->getCamera().getView(), m_scene->getCamera().getProjection(m_window->aspect()));

        m_scene->update(m_deltaTime);
    }

    void Editor::draw() {
        core::UIImGui::newFrame();

        if (m_imguiDemo) {
            ImGui::ShowDemoWindow(&m_imguiDemo);
            m_widowOpen = !m_widowOpen;
        }

        menuBar();
        entitiesPanel();

        if (m_cameraControls) cameraControls();

        if (m_addEntity) addEntity();

        if (m_addModel) addModel();

        if (m_saveScene) saveScene();

        drawGizmo();

        core::UIImGui::render();
    }

    void Editor::cleanup() {

    }

    void Editor::menuBar() {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                ImGui::MenuItem("New");
                if (ImGui::MenuItem("Save")) m_widowOpen = m_saveScene = !m_saveScene;
                ImGui::MenuItem("Open");

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Entity")) {
                if (ImGui::MenuItem("Add Entity")) m_addEntity = !m_addEntity;

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Assets")) {
                if (ImGui::MenuItem("Add Model")) m_widowOpen = m_addModel = !m_addModel;

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Tools")) {
                if (ImGui::MenuItem("Camera Controls", nullptr)) m_cameraControls = !m_cameraControls;
                if (ImGui::MenuItem("ImGui Demo", nullptr)) m_widowOpen = m_imguiDemo = !m_imguiDemo;

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
            for (size_t i = 0; i < m_entitiesInfo.size(); ++i) {
                char buf[32];

                sprintf(buf, "%s", m_entitiesInfo[i].name.c_str());

                if (ImGui::Selectable(buf, m_entitySelected == i)) m_entitySelected = i;
            }

        }
        ImGui::End();

        ImGui::SetNextWindowPos({0.0f, io.DisplaySize.y * 0.5f});
        ImGui::SetNextWindowSize({350.0f, io.DisplaySize.y * 0.5f});
        ImGui::Begin("Entity Properties", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
        {
            if (m_entitySelected != -1) {
                auto& entity = m_scene->getEntity(m_entitySelected);
                auto& transform = m_scene->getComponent<core::Transform>(entity.id);

                ImGui::Text("Entity ID: %i", entity.id);

                ImGui::InputText("Entity name", entity.name.data(), 30);
                m_entitiesInfo[m_entitySelected].name = entity.name;

                ImGui::Text("Flags %lu", entity.flags);

                std::vector<const char*> entityTypes = {"CAMERA", "PLAYER"};

                if (entity.flags & core::EntityFlags::CAMERA) {
                    if (ImGui::CollapsingHeader("Camera")) {
                        auto& camera = m_scene->getComponent<core::Camera>(entity.id);

                        ImGui::InputFloat3("Eye", glm::value_ptr(camera.getEye()));
                        ImGui::InputFloat3("Front", glm::value_ptr(camera.getCenter()));
                        ImGui::Separator();

                        glm::vec2 angles = glm::degrees(camera.getEulerAngles());
                        ImGui::InputFloat2("Euler Angles", glm::value_ptr(angles));
                        camera.getEulerAngles() = glm::radians(angles);

                        ImGui::Separator();
                        ImGui::InputFloat("FOV", &camera.getFovy());
                        ImGui::Separator();
                        ImGui::InputFloat("Velocity", &camera.getSpeed());
                        ImGui::InputFloat("Turn Velocity", &camera.getTurnSpeed());
                        ImGui::Separator();
                        ImGui::InputFloat("Distances", &camera.getDistance());
                    }
                } else {
                    if (ImGui::CollapsingHeader("Transform")) {

                        ImGui::InputFloat3("Position", glm::value_ptr(transform.getPosition()));
                        ImGui::InputFloat3("Size", glm::value_ptr(transform.getSize()));

                        glm::vec3 angles = glm::degrees(transform.getRotation());
                        ImGui::InputFloat3("Rotation", glm::value_ptr(angles));
                        transform.getRotation() = glm::radians(angles);

                        ImGui::InputFloat("Velocity", &transform.getSpeed());
                    }
                }

                if (entity.components & core::MODEL) {
                    if (ImGui::CollapsingHeader("Model")) {
                        auto& meshModel = m_scene->getComponent<core::MeshModel>(entity.id);

                        ImGui::Text("Model ID: %lu", meshModel.getModelID());

                        int currentModel = m_entitiesInfo[m_entitySelected].model;

                        if (ImGui::BeginCombo("Name", m_modelsNames[currentModel].c_str())) {
                            for (int i = 0; i < m_modelsNames.size(); ++i) {
                                const bool is_selected = (currentModel == i);

                                if (ImGui::Selectable(m_modelsNames[i].c_str(), is_selected)) {
                                    m_entitiesInfo[m_entitySelected].model = currentModel = i;
                                    uint64_t modelID = core::tools::hashString(m_modelsNames[currentModel]);
                                    meshModel.setModelID(modelID);

                                    auto& model = resourceManager->getModel(modelID);
                                    auto& node = model.getBaseMesh();

                                    glm::vec3 tempTranslate, tempScale, tempSkew;
                                    glm::vec4 tempPerspective;
                                    glm::quat tempOrientation;

                                    glm::decompose(node.matrix, tempScale, tempOrientation, tempTranslate, tempSkew, tempPerspective);

                                    transform.getPosition() += tempTranslate;
                                    transform.getRotation() += glm::eulerAngles(tempOrientation);
                                }

                                if (is_selected) ImGui::SetItemDefaultFocus();
                            }

                            ImGui::EndCombo();
                        }

                        if (ImGui::CollapsingHeader(m_modelsNames[m_entitiesInfo[m_entitySelected].model].c_str())) {
                            auto& model = resourceManager->getModel(meshModel.getModelID());

                            loadNode(model.getNode(0), model);
                        }
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
            ImGui::InputFloat("Velocity", &m_scene->getCamera().getSpeed());
            ImGui::InputFloat("Turn Velocity", &m_scene->getCamera().getTurnSpeed());
            ImGui::Separator();
            ImGui::InputFloat("Distances", &m_scene->getCamera().getDistance());
        }
        ImGui::End();
    }

    void Editor::cameraMovement() {
        auto& camera = m_scene->getCamera();

        if (!m_widowOpen)
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

        if (m_entitySelected != -1) {
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

            auto& transform = m_scene->getComponent<core::Transform>(m_entitySelected);
            editor::gizmo::transform(transform, m_currentOperation, m_scene->getCamera().getView(), projMatrix);
        }
    }

    void Editor::addEntity() {
        auto& entity = m_scene->addEntity("Object", core::EntityFlags::OBJECT);
        uint64_t modelID = core::tools::hashString("cube");

        m_scene->registry().emplace<core::MeshModel>(entity.enttID, modelID);
        m_scene->registry().emplace<core::Transform>(entity.enttID, m_scene->getCamera().getCenter(), DEFAULT_SIZE, SPEED_ZERO, DEFAULT_ROTATION);
        entity.components = core::MODEL | core::TRANSFORM;

        m_entitiesInfo.push_back({entity.id, entity.name, 0});
        m_addEntity = !m_addEntity;
    }

    void Editor::addModel() {
        if (m_addModel)
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".gltf", "../assets/");

        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
                int idx = filePathName.rfind('/');
                std::string fileName = filePathName.substr(idx + 1, filePathName.size());

                idx = fileName.rfind('.');
                std::string modelName = fileName.substr(0, idx);

                resourceManager->createModel(fileName, modelName);
                m_modelsNames.emplace_back(modelName);
            }

            ImGuiFileDialog::Instance()->Close();
            m_widowOpen = m_addModel = !m_addModel;
        }
    }

    void Editor::loadNode(core::Model::Node& node, core::Model& model) {
        if (ImGui::TreeNode(node.name.c_str())) {
            ImGui::Text("ID: %u", node.id);

            if (ImGui::CollapsingHeader("Data")) {
                std::string matrix;
                for (int i = 0; i < 4; ++i) {
                    matrix.append("| ");

                    for (int j = 0; j < 4; ++j) {
                        matrix.append(std::to_string(node.matrix[i][j]) + ' ');
                    }

                    matrix.append("|\n");
                }
                ImGui::Text("Matrix: \n%s", matrix.c_str());

                ImGui::Text("Mesh ID: %lu", node.mesh);
            }

            for (auto& child : node.children) {
                loadNode(model.getNode(child), model);
            }

            ImGui::TreePop();
        }
    }

    void Editor::saveScene() {
        if (m_saveScene)
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose a Directory", nullptr, "../data/");

        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
                filePath.append(".json");

                m_scene->saveScene(filePath, true);
            }

            ImGuiFileDialog::Instance()->Close();
            m_widowOpen = m_saveScene = !m_saveScene;
        }
    }

} // namespace editor