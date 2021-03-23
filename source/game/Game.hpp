#ifndef PROTOTYPE_ACTION_RPG_GAME_HPP
#define PROTOTYPE_ACTION_RPG_GAME_HPP

#include "Application.hpp"

namespace game {

    class Game : public core::Application {
    public:
        Game();

        void init() override;

        void update() override;

        void draw() override;

        void cleanup() override;
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_GAME_HPP
