#include "Game.hpp"

#include "glm/gtc/type_ptr.hpp"


namespace game {

    Game::Game() : core::Application("Action RPG") {

    }

    void Game::init() {
        m_scene->loadScene("../data/scene.json");
    }

    void Game::update() {
        renderer->updateVP(m_scene->getCamera().getView(), m_scene->getCamera().getProjection(m_window->aspect()));

        m_scene->update(m_deltaTime);
    }

    void Game::draw() {
        core::UIImGui::newFrame();

        ImGui::InputFloat3("Eye", glm::value_ptr(m_scene->getCamera().getEye()));
        ImGui::InputFloat3("Front", glm::value_ptr(m_scene->getCamera().getCenter()));
        ImGui::InputFloat3("Up", glm::value_ptr(m_scene->getCamera().getUp()));
        ImGui::Separator();
        ImGui::InputFloat2("Euler Angles", glm::value_ptr(m_scene->getCamera().getEulerAngles()));
        ImGui::Separator();
        ImGui::InputFloat("FOV", &m_scene->getCamera().getFovy());
        ImGui::Separator();
        ImGui::InputFloat("Velocity", &m_scene->getCamera().getSpeed());
        ImGui::InputFloat("Turn Velocity", &m_scene->getCamera().getTurnSpeed());
        ImGui::Separator();
        ImGui::InputFloat("Distances", &m_scene->getCamera().getDistance());

        core::UIImGui::render();
    }

    void Game::cleanup() {

    }

} // namespace core