#include "Game.hpp"

#include "glm/gtc/type_ptr.hpp"


namespace game {

    Game::Game() : engine::Application("Action RPG") {

    }

    void Game::init() {
        m_scene->loadScene("../data/scene.json");

        m_luaManager.setScriptsDir("game");
        m_luaManager.scriptFile("main.lua");
        m_luaManager.executeFunction("init");
    }

    void Game::update() {

    }

    void Game::drawUI() {

    }

    void Game::cleanup() {

    }

    void Game::renderCommands(vk::CommandBuffer &cmdBuffer) {

    }

} // namespace core