#ifndef PROTOTYPE_ACTION_RPG_GIZMOS_HPP
#define PROTOTYPE_ACTION_RPG_GIZMOS_HPP


#include "glm/glm.hpp"
#include "imgui.h"
#include "ImGuizmo.h"


#include "components/Transform.hpp"


namespace editor {

    namespace gizmo {

        void transform(core::Transform& transform, ImGuizmo::OPERATION operation, const glm::mat4& view, const glm::mat4& proj);

    } // namespace gizmo

} // namespace editor


#endif //PROTOTYPE_ACTION_RPG_GIZMOS_HPP
