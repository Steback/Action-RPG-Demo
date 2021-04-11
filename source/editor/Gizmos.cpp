#include "Gizmos.hpp"


#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/matrix_decompose.hpp"


namespace editor {

    namespace gizmo {

        void transform(engine::Transform& transform, ImGuizmo::OPERATION operation, const glm::mat4& view, const glm::mat4& proj) {
            ImGuiIO& io = ImGui::GetIO();

            glm::mat4 modelMatrix;
            glm::mat4 deltaMatrix;

            ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

            ImGuizmo::RecomposeMatrixFromComponents(glm::value_ptr(transform.getPosition()),
                                                    glm::value_ptr(transform.getRotation()),
                                                    glm::value_ptr(transform.getSize()),
                                                    glm::value_ptr(modelMatrix));

            ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj), operation,ImGuizmo::MODE::WORLD,
                                 glm::value_ptr(modelMatrix), glm::value_ptr(deltaMatrix), nullptr);

            glm::vec3 tempTranslate, tempScale, tempSkew;
            glm::vec4 tempPerspective;
            glm::quat tempOrientation;

            glm::decompose(deltaMatrix, tempScale, tempOrientation, tempTranslate, tempSkew, tempPerspective);

            switch (operation) {
                case ImGuizmo::OPERATION::TRANSLATE:
                    if (tempTranslate != glm::vec3(0, 0, 0)) {
                        transform.getPosition() += tempTranslate;
                    }

                    break;
                case ImGuizmo::ROTATE:
                    if (tempOrientation != glm::quat(1,0,0,0)) {
                        transform.getRotation() += glm::eulerAngles(tempOrientation);
                    }

                    break;
                case ImGuizmo::SCALE:
                    if (tempScale != glm::vec3(1, 1, 1)) {
                        transform.getSize() *= tempScale;
                    }

                    break;
            }
        }

    } // namespace gizmo

} // namespace editor