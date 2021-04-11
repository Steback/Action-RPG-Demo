#ifndef PROTOTYPE_ACTION_RPG_EDITOR_HPP
#define PROTOTYPE_ACTION_RPG_EDITOR_HPP


#include "imgui.h"
#include "ImGuizmos/ImGuizmo.h"

#include "Application.hpp"
#include "renderer/GraphicsPipeline.hpp"


namespace editor {

    struct EntityInfo {
        uint32_t entityID;
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

    private:
        void menuBar();

        void entitiesPanel();

        void cameraControls();

        void cameraMovement();

        void drawGizmo();

        void addEntity();

        void addModel();

        void loadNode(engine::ModelInterface::Node& node, engine::ModelInterface& model);

        void saveScene();

        void loadScene();

    public:
        void renderCommands(vk::CommandBuffer &cmdBuffer) override;

    private:
        size_t m_entitySelected = -1;
        bool m_gizmoDraw = true;
        bool m_cameraControls = false;
        ImGuizmo::OPERATION m_currentOperation;
        bool m_addEntity = false;
        bool m_imguiDemo = false;
        bool m_addModel = false;
        bool m_widowOpen = false;
        bool m_saveScene = false;
        bool m_loadScene = false;
        std::vector<std::string> m_modelsNames;
        std::vector<EntityInfo> m_entitiesInfo;
        std::string m_sceneName{};
        bool m_sceneLoaded =  false;
        std::shared_ptr<engine::GraphicsPipeline> m_gridPipeline;
    };

} // namespace editor


#endif //PROTOTYPE_ACTION_RPG_EDITOR_HPP
