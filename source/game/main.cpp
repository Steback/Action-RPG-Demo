#include "spdlog/spdlog.h"

#include "Game.hpp"

int main() {
    game::Game game;

    try {
        game.run();
    } catch (const std::exception& ex) {
        spdlog::error("{}", ex.what());
    }

    return EXIT_SUCCESS;
}


