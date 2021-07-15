#include "Combat.hpp"


namespace game {

    Combat::Combat() = default;

    Combat::Combat(unsigned int health, unsigned int damage, unsigned int armor)
            : health(health), damage(damage), armor(armor) {

    }

    void Combat::calcDamage(const Combat &enemy) {
        health -= (enemy.damage - armor);
    }

    void Combat::update() {
        if (health <= 0) {
            isAlive = false;
            health = 0;
        }
    }

    void Combat::setLuaBindings(sol::table &table) {
        table.new_usertype<Combat>("table",
                                   "health", &Combat::health,
                                   "damage", &Combat::damage,
                                   "armor", &Combat::armor);
    }

} // namespace game
