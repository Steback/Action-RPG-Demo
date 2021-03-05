#ifndef PROTOTYPE_ACTION_RPG_EDITOR_HPP
#define PROTOTYPE_ACTION_RPG_EDITOR_HPP



#include "imgui.h"
#include "ImGuizmo.h"

#include "Application.hpp"


namespace editor {

    class Editor : public core::Application {
    public:
        Editor();

        ~Editor();

        void init() override;

        void update() override;

        void draw() override;

        void cleanup() override;

        void menuBar();

        void entitiesPanel();

        void cameraControls();

        void cameraMovement();

        void drawGizmo();

    private:
        size_t entitySelected = -1;
        bool m_gizmoDraw = true;
        bool m_cameraControls = false;
        ImGuizmo::OPERATION m_currentOperation;
    };

} // namespace editor


#endif //PROTOTYPE_ACTION_RPG_EDITOR_HPP
