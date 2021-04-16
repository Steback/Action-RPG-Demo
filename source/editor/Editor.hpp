#ifndef PROTOTYPE_ACTION_RPG_EDITOR_HPP
#define PROTOTYPE_ACTION_RPG_EDITOR_HPP

#include "imgui.h"
#include "ImGuizmos/ImGuizmo.h"

#include "Application.hpp"
#include "renderer/GraphicsPipeline.hpp"

namespace engine::ui {
    class Window;
}

namespace editor {

    struct MenuItem {
        std::string name;
        sol::function func;
    };

    struct MenuBar {
        std::string name;
        std::vector<MenuItem> items;
    };

    struct EntityInfo {
        uint32_t id;
        std::string name;
        int model;
    };

    class Editor : public engine::Application {
    public:
        Editor();

        ~Editor();

        void init() override;

        void update() override;

        void drawUI() override;

        void cleanup() override;

        void renderCommands(vk::CommandBuffer &cmdBuffer) override;

        void loadMenuBar(sol::state& state);

        void setLuaBindings(sol::table& state);

    private:
        void menuBar();

        void cameraMovement();

        void drawGizmo();

        void addEntity(const std::string& name,  const std::string& model);

        void addModel(const std::string& title, const std::string& filters, const std::string& path, const std::string& openName);

        void saveScene(const std::string& title, const std::string& filters, const std::string& path, const std::string& openName);

        void loadScene(const std::string& title, const std::string& filters, const std::string& path, const std::string& openName);

        void showImGuiDemo(const std::string& openName);

        [[nodiscard]] int getEntity() const;

        void setEntity(int entity);

    private:
        size_t m_entitySelected = -1;
        bool m_gizmoDraw = true;
        ImGuizmo::OPERATION m_currentOperation;
        bool m_widowOpen = false;
        std::vector<std::string> m_modelsNames;
        std::vector<EntityInfo> m_entitiesInfo;
        std::string m_sceneName{};
        bool m_sceneLoaded =  false;
        std::shared_ptr<engine::GraphicsPipeline> m_gridPipeline;
        std::vector<MenuBar> m_menuBar;
    };

} // namespace editor


#endif //PROTOTYPE_ACTION_RPG_EDITOR_HPP
