#include "ImGuiBindings.hpp"

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"
#include "imgui.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "fmt/format.h"

#include "LuaManager.hpp"


namespace engine::lua {

    void setImGuiBindings(sol::state& state) {
        sol::table imgui = state.create_table("imgui");

        imgui.new_enum("flags",
                       "noResize", ImGuiWindowFlags_NoResize,
                       "noCollapse", ImGuiWindowFlags_NoCollapse);

        imgui.set_function("text", &ImGui::Text);
        imgui.set_function("separator", &ImGui::Separator);
        imgui.set_function("button", [](const std::string& label) {return ImGui::Button(label.c_str()); });
        imgui.set_function("collapsingHeader", [](const std::string& title){ return ImGui::CollapsingHeader(title.c_str()); });
        imgui.set_function("selectable", [](const std::string& text, bool comapration){ return ImGui::Selectable(text.c_str(), comapration); });
        imgui.set_function("inputFloat3", [](const std::string& title, glm::vec3& v){ ImGui::InputFloat3(title.c_str(), glm::value_ptr(v)); });
        imgui.set_function("inputFloat2", [](const std::string& title, glm::vec2& v){ ImGui::InputFloat2(title.c_str(), glm::value_ptr(v)); });
        imgui.set_function("inputFloat4", [](const std::string& title, glm::quat& q){ ImGui::InputFloat4(title.c_str(), glm::value_ptr(q)); });
        imgui.set_function("inputFloat", [](const std::string& title, float* n){ ImGui::InputFloat(title.c_str(), n); });
        imgui.set_function("getFrameRate", [](){ return ImGui::GetIO().Framerate; });
        imgui.set_function("progressBar", [](float fraction, float width, float height){ ImGui::ProgressBar(fraction, {width, height}); });
        imgui.set_function("inputText", [](const std::string& title, const std::string& name){
            char* tempName = const_cast<char *>(name.c_str());
            ImGui::InputText(title.c_str(), tempName, 30);

            return tempName;
        });
        imgui.set_function("beginCombo",[](const std::string& title, const std::string& first, const sol::function& f) {
            if (ImGui::BeginCombo(title.c_str(), first.c_str())) {
                LuaManager::executeFunction(f);
                ImGui::EndCombo();
            }
        });
        imgui.set_function("checkBox", [](const std::string& label, bool check){
            ImGui::Checkbox(label.c_str(), &check);

            return check;
        });
        imgui.set_function("treeNode", [](const std::string& label, const sol::function& f){
            if (ImGui::TreeNode(label.c_str())) {
                LuaManager::executeFunction(f);
                ImGui::TreePop();
            }
        });
    }
}