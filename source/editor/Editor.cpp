#include "Editor.hpp"

#include "imgui.h"
#include "fmt/format.h"
#include "ImGuiFileDialog/CustomImGuiFileDialogConfig.h"
#include "ImGuiFileDialog/ImGuiFileDialog.h"

#include "components/ModelInterface.hpp"
#include "Gizmos.hpp"
#include "ui/Window.hpp"


namespace editor {

    Editor::Editor() : engine::Application("Editor", {0.24f, 0.24f, 0.24f, 1.0f}, true),
            m_currentOperation(ImGuizmo::OPERATION::TRANSLATE) {

    }

    Editor::~Editor() = default;

    void Editor::init() {
        vk::PushConstantRange constantRange{
                .stageFlags = vk::ShaderStageFlagBits::eVertex,
                .offset = 0,
                .size = sizeof(engine::MVP)
        };

        m_gridPipeline = m_renderer->addPipeline(engine::Application::m_resourceManager->createShader("grid.vert.spv", "grid.frag.spv", {constantRange}, false),
                                                 m_device->m_logicalDevice, nullptr, true);

        m_resourceManager->createModel("cube.json", "cube");
        m_modelsNames.emplace_back("cube");

        m_scene->loadScene("../data/basicScene.json", true);

        for (auto& entity : m_scene->getEntities()) {
            auto& model = m_scene->getComponent<engine::ModelInterface>(entity.id);
            int modelID;

            for (int i = 0; i < m_modelsNames.size(); ++i) {
                if (m_modelsNames[i] == model.getName()) modelID = i;
            }

            m_entitiesInfo.push_back({entity.id, entity.name, modelID});
        }

        m_scene->getCamera() = engine::Camera({45.0f, 45.0f}, {0.0f, 0.0f, 0.0f}, 0.5f, 10.0f, 10.0f);

        // LUA
        m_luaManager.setScriptsDir("editor");
        auto imgui = m_luaManager.get<sol::table>("imgui");
        imgui.set_function("showDemo", &Editor::showImGuiDemo, this);

        sol::table editor = m_luaManager.getState().create_table("editor");
        setLuaBindings(editor);

        m_luaManager.scriptFile("main.lua");

        loadMenuBar(m_luaManager.getState());
        m_luaManager.executeFunction("init");

        auto windowSize = m_window->getSize();
    }

    void Editor::update() {
        cameraMovement();

        m_renderer->updateVP(m_scene->getCamera().getView(), m_scene->getCamera().getProjection(m_window->aspect()));

        m_scene->update(m_deltaTime);
    }

    void Editor::drawUI() {
        menuBar();
        drawGizmo();
    }

    void Editor::cleanup() {

    }

    void Editor::menuBar() {
        if (ImGui::BeginMainMenuBar()) {
            for (auto& menuBar : m_menuBar) {
                if (ImGui::BeginMenu(menuBar.name.c_str())) {
                    for (auto& item : menuBar.items) {
                        if (ImGui::MenuItem(item.name.c_str())) engine::LuaManager::executeFunction(item.func);
                    }

                    ImGui::EndMenu();
                }
            }
        }
    }

    void Editor::cameraMovement() {
        auto& camera = m_scene->getCamera();

//        if (!m_widowOpen)
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

            auto& transform = m_scene->getComponent<engine::Transform>(m_entitySelected);
            editor::gizmo::transform(transform, m_currentOperation, m_scene->getCamera().getView(), projMatrix);
        }
    }

    void Editor::addEntity(const std::string& name,  const std::string& model) {
        auto& entity = m_scene->addEntity(name, engine::EntityType::OBJECT);
        uint64_t modelID = engine::tools::hashString(model);

        m_scene->registry().emplace<engine::ModelInterface>(entity.enttID, modelID, entity.id);
        m_scene->registry().emplace<engine::Transform>(entity.enttID, m_scene->getCamera().getCenter(), DEFAULT_SIZE, SPEED_ZERO, DEFAULT_ROTATION);
        entity.components = engine::MODEL | engine::TRANSFORM;

        m_entitiesInfo.push_back({entity.id, entity.name, 0});
    }

    void Editor::addModel(const std::string& title, const std::string& filters, const std::string& path, const std::string& openName) {
        bool open = m_luaManager.get<bool>(openName);

        ImGui::SetNextWindowSize({500, 250});

        if (open)
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", title, filters.c_str(), path);

        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                int idx = (int)filePathName.rfind('/');
                std::string fileName = filePathName.substr(idx + 1, filePathName.size());

                idx = (int)fileName.rfind('.');
                std::string modelName = fileName.substr(0, idx);

                m_resourceManager->createModel(fileName, modelName);
                m_modelsNames.emplace_back(modelName);
            }

            ImGuiFileDialog::Instance()->Close();

            m_luaManager.getState()[openName] = !open;
        }
    }

    void Editor::saveScene(const std::string& title, const std::string& filters, const std::string& path, const std::string& openName) {
        bool open = m_luaManager.get<bool>(openName);

        if (m_sceneLoaded) {
            m_scene->saveScene(m_sceneName, true);
            return ;
        }

        ImGui::SetNextWindowSize({500, 250});

        if (open)
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", title, filters.c_str(), path);

        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();

                m_threadPool->submit([scene = m_scene.get(), filePathName] {
                    scene->saveScene(filePathName, true);
                });
            }

            ImGuiFileDialog::Instance()->Close();

            m_luaManager.getState()[openName] = !open;
        }
    }

    void Editor::loadScene(const std::string& title, const std::string& filters, const std::string& path, const std::string& openName, const std::string& selected) {
        bool open = m_luaManager.get<bool>(openName);

        ImGui::SetNextWindowSize({500, 250});

        if (open)
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", title, filters.c_str(), path);

        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                m_luaManager.getState()[selected] = true;
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();

                m_threadPool->submit([path = filePathName, entitiesInfo = &m_entitiesInfo, name = &m_sceneName, loaded = &m_sceneLoaded, scene = m_scene.get(), modelNames = &m_modelsNames]{
                    int idx = (int)path.rfind('/');
                    std::string fileName = path.substr(idx + 1, path.size());

                    entitiesInfo->clear();
                    scene->loadScene(path, true, modelNames);

                    for (auto& entity : m_scene->getEntities()) {
                        auto& model = m_scene->getComponent<engine::ModelInterface>(entity.id);
                        int modelID;

                        for (int i = 0; i < modelNames->size(); ++i) {
                            if (modelNames->at(i) == model.getName()) modelID = i;
                        }

                        entitiesInfo->push_back({entity.id, entity.name, modelID});
                    }

                    *name = path;
                    *loaded = true;
                });
            }

            ImGuiFileDialog::Instance()->Close();
            m_luaManager.getState()[openName] = !open;
        }
    }

    void Editor::renderCommands(vk::CommandBuffer &cmdBuffer) {
        m_gridPipeline->bind(cmdBuffer);

        engine::MVP mvp = m_renderer->m_mvp;
        mvp.model = glm::mat4(1.0f);

        cmdBuffer.pushConstants(m_gridPipeline->getLayout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(mvp), &mvp);
        cmdBuffer.draw(6, 1, 0, 0);
    }

    void Editor::loadMenuBar(sol::state& state) {
        sol::table menuBarTable = state["menuBar"];

        uint menuIndex = 0;
        while (true) {
            sol::optional<sol::table> existsMenu = menuBarTable[menuIndex];

            if (existsMenu == sol::nullopt) {
                break;
            } else {
                sol::table menu = existsMenu.value();
                MenuBar menuBar{
                    .name = menu.get<std::string>("name")
                };

                uint itemIndex = 0;
                while (true) {
                    sol::optional<sol::table> existsItem = menu["items"][itemIndex];

                    if (existsItem == sol::nullopt) {
                        break;
                    } else {
                        sol::table menuItem = existsItem.value();

                        menuBar.items.push_back({
                            .name = menuItem.get<std::string>("name"),
                            .func = menuItem.get<sol::function>("func"),
                        });
                    }

                    ++itemIndex;
                }

                m_menuBar.push_back(menuBar);
            }

            ++menuIndex;
        }
    }

    void Editor::setLuaBindings(sol::table &state) {
        state.new_usertype<EntityInfo>("EntityInfo",
                                       "id", &EntityInfo::id,
                                       "name", &EntityInfo::name,
                                       "model", &EntityInfo::model);

        state["entitiesInfo"] = std::ref(m_entitiesInfo);
        state["modelsNames"] = std::ref(m_modelsNames);

        state.set_function("saveScene", &Editor::saveScene, this);
        state.set_function("loadScene", &Editor::loadScene, this);
        state.set_function("addEntity", &Editor::addEntity, this);
        state.set_function("addModel", &Editor::addModel, this);
        state.set_function("getEntity", &Editor::getEntity, this);
        state.set_function("setEntity", &Editor::setEntity, this);
    }

    int Editor::getEntity() const {
        return static_cast<int>(m_entitySelected);
    }

    void Editor::setEntity(int entity) {
        m_entitySelected = entity - 1;
    }

    void Editor::showImGuiDemo(const std::string &openName) {
        bool open = m_luaManager.get<bool>(openName);

        ImGui::ShowDemoWindow(&open);

        m_luaManager.getState()[openName] = open;
    }

} // namespace editor