#ifndef ACTION_RPG_DEMO_SOURCE_GAME_COMBATSYSTEM_HPP
#define ACTION_RPG_DEMO_SOURCE_GAME_COMBATSYSTEM_HPP


#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

#include "scene/Scene.hpp"
#include "components/Combat.hpp"


namespace game {

    class CombatSystem {
    public:
        CombatSystem();

        ~CombatSystem();

        engine::Entity& getHero();

        engine::Entity& getEnemy();

        void update();

        void setLuaBindings(sol::table game);

    private:
        static void calculateDamage(Combat& obj1, Combat& obj2);

    private:
        engine::Entity hero{};
        engine::Entity enemy{};
        bool contact{};
    };

} // namespace game


#endif //ACTION_RPG_DEMO_SOURCE_GAME_COMBATSYSTEM_HPP
