#include "Game.hpp"

#include "glm/gtc/type_ptr.hpp"


namespace game {

    Game::Game() : engine::Application("Action RPG") {

    }

    void Game::init() {
        m_scene->loadScene("../data/scene.json");
    }

    void Game::update() {

    }

    void Game::drawUI() {
        ImGui::SetNextWindowSize({-1, -1});
        ImGui::Begin("Debug", nullptr);
        {
            ImGui::Text("Delta time: %f", m_deltaTime);
            ImGui::Text("Delta time: %f", 1 / 60.0f);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }
        ImGui::End();
    }

    void Game::cleanup() {

    }

    void Game::renderCommands(vk::CommandBuffer &cmdBuffer) {

    }

} // namespace core