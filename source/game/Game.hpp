#ifndef PROTOTYPE_ACTION_RPG_GAME_HPP
#define PROTOTYPE_ACTION_RPG_GAME_HPP

#include "Application.hpp"

namespace game {

    class Game : public engine::Application {
    public:
        Game();

        void init() override;

        void update() override;

        void drawUI() override;

        void cleanup() override;

        void renderCommands(vk::CommandBuffer &cmdBuffer) override;
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_GAME_HPP
