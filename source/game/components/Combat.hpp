#ifndef ACTION_RPG_DEMO_SOURCE_GAME_COMPONENTS_COMBAT_HPP
#define ACTION_RPG_DEMO_SOURCE_GAME_COMPONENTS_COMBAT_HPP


#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"


namespace game {

    class Combat {
    public:
        Combat();

        Combat(unsigned int health, unsigned int damage, unsigned int armor);

        void calcDamage(const Combat& enemy);

        void update();

        static void setLuaBindings(sol::table& table);

    public:
        unsigned int health{};
        unsigned int damage{};
        unsigned int armor{};
        bool isAlive{true};
        bool isCombating{};
    };

} // namespace game


#endif //ACTION_RPG_DEMO_SOURCE_GAME_COMPONENTS_COMBAT_HPP
