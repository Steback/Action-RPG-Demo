#ifndef PROTOTYPE_ACTION_RPG_EDITOR_HPP
#define PROTOTYPE_ACTION_RPG_EDITOR_HPP


#include "imgui.h"
#include "ImGuizmo.h"
#include "ImGuiFileDialog.h"

#include "Application.hpp"


namespace editor {

    struct EntityInfo {
        uint32_t entityID;
        std::string name;
        int model;
    };

    class Editor : public core::Application {
    public:
        Editor();

        ~Editor();

        void init() override;

        void update() override;

        void draw() override;

        void cleanup() override;

    private:
        void menuBar();

        void entitiesPanel();

        void cameraControls();

        void cameraMovement();

        void drawGizmo();

        void addEntity();

        void addModel();

        void loadNode(core::Model::Node& node, core::Model& model);

    private:
        size_t m_entitySelected = -1;
        bool m_gizmoDraw = true;
        bool m_cameraControls = false;
        ImGuizmo::OPERATION m_currentOperation;
        bool m_addEntity = false;
        bool m_imguiDemo = false;
        bool m_addModel = false;
        bool m_widowOpen = false;
        std::vector<std::string> m_modelsNames;
        std::vector<EntityInfo> m_entitiesInfo;
    };

} // namespace editor


#endif //PROTOTYPE_ACTION_RPG_EDITOR_HPP
