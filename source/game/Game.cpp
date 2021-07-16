#include "Game.hpp"

#include "glm/gtc/type_ptr.hpp"


namespace game {

    Game::Game() : engine::Application("Action RPG") {

    }

    void Game::init() {
        m_scene->loadScene("..\\..\\data\\scene.json");

        m_luaManager.setScriptsDir("game");
        m_luaManager.scriptFile("main.lua");
        m_luaManager.executeFunction("init");

        sol::table components = m_luaManager.getState().get<sol::table>("components");
        Combat::setLuaBindings(components);
        combatSystem = std::make_unique<CombatSystem>();

        sol::table game = m_luaManager.getState()["game"].get_or_create<sol::table>();
        combatSystem->setLuaBindings(game);
        game.set_function("getCombatComponent", &Game::getCombatComponent, this);
    }

    void Game::update() {
        combatSystem->update();

        auto viewCombat = m_scene->registry().view<Combat>();
        for (auto& entity : viewCombat)
            viewCombat.get<Combat>(entity).update();
    }

    void Game::drawUI() {

    }

    void Game::cleanup() {

    }

    void Game::renderCommands(vk::CommandBuffer &cmdBuffer) {

    }

    Combat& Game::getCombatComponent(uint32_t id) {
        return m_scene->getComponent<Combat>(id);
    }

} // namespace core