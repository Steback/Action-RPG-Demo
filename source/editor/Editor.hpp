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

        void drawGizmo();

    private:
        size_t entitySelected = -1;
        glm::mat4 m_proj{};
        bool m_gizmoDraw = true;
        ImGuizmo::OPERATION m_currentOperation;
    };

} // namespace editor


#endif //PROTOTYPE_ACTION_RPG_EDITOR_HPP
