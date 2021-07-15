#ifndef PROTOTYPE_ACTION_RPG_GAME_HPP
#define PROTOTYPE_ACTION_RPG_GAME_HPP


#include "Application.hpp"
#include "CombatSystem.hpp"
#include "components/Combat.hpp"


namespace game {

    class Game : public engine::Application {
    public:
        Game();

        void init() override;

        void update() override;

        void drawUI() override;

        void cleanup() override;

        void renderCommands(vk::CommandBuffer &cmdBuffer) override;

    private:
        Combat& getCombatComponent(uint32_t id);

    private:
        std::unique_ptr<CombatSystem> combatSystem;
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_GAME_HPP
