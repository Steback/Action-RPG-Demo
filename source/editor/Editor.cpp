#include "Editor.hpp"

#include "imgui.h"
#include "glm/gtc/type_ptr.hpp"

#include "renderer/UIImGui.hpp"


namespace editor {

    Editor::Editor() : core::Application("Editor") {

    }

    Editor::~Editor() = default;

    void Editor::init() {
        m_resourceManager->createTexture("plain.png");
        auto enttID = m_registry.create();
        auto entity = m_scene->addEntity("Viking Room", enttID);

        m_registry.emplace<core::Transform>(enttID, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, glm::vec3(0.0f));
        m_registry.emplace<core::Model>(enttID, m_resourceManager->createModel("viking-room.gltf"));
    }

    void Editor::update() {

    }

    void Editor::draw() {
        core::UIImGui::newFrame();

        ImGui::ShowDemoWindow();

        menuBar();
        entitiesPanel();

        core::UIImGui::render();
    }

    void Editor::cleanup() {
        auto view = m_registry.view<core::Model>();

        for (auto& entity : view) {
            auto& model = view.get<core::Model>(entity);
            model.clean();
        }
    }

    void Editor::menuBar() {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                ImGui::MenuItem("Save", nullptr, nullptr);
            }
        }
    }

    void Editor::entitiesPanel() {
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        ImGui::SetNextWindowPos({0.0f, 22});
        ImGui::SetNextWindowSize({350.0f, ((io.DisplaySize.y * 0.5f) - 22)});
        ImGui::Begin("Entities", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
        {
            for (size_t i = 0; i < m_scene->getEntitiesCount(); ++i) {
                auto entity = m_scene->getEntity(i);
                char buf[32];

                sprintf(buf, "%s", entity.name.c_str());

                if (ImGui::Selectable(buf, entitySelected == i)) entitySelected = i;
            }

        }
        ImGui::End();

        ImGui::SetNextWindowPos({0.0f, io.DisplaySize.y * 0.5f});
        ImGui::SetNextWindowSize({350.0f, io.DisplaySize.y * 0.5f});
        ImGui::Begin("Entity Properties", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
        {
            if (entitySelected != -1) {
                auto& entity = m_scene->getEntity(entitySelected);

                ImGui::Text("Entity ID: %i", entity.id);

                char* entityName = entity.name.data();
                ImGui::InputText("Entity name", entityName, 30);

                entity.name = entityName;

                if (ImGui::CollapsingHeader("Transform")) {
                    auto& transform = m_registry.get<core::Transform>(entity.enttID);

                    ImGui::InputFloat3("Position", glm::value_ptr(transform.getPosition()));
                    ImGui::InputFloat3("Size", glm::value_ptr(transform.getSize()));
                    ImGui::InputFloat3("Rotation", glm::value_ptr(transform.getRotation()));
                    ImGui::InputFloat("Velocity", &transform.getVelocity());
                }
            }
        }
        ImGui::End();
    }

} // namespace editor