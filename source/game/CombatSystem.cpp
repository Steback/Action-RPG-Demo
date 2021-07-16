#include "CombatSystem.hpp"

#include "nlohmann/json.hpp"

#include "Game.hpp"
#include "components/Status.hpp"
#include "components/Movement.hpp"
#include "components/Transform.hpp"
#include "components/AnimationInterface.hpp"
#include "resources/Animation.hpp"
#include "resources/Animation.hpp"


using json = nlohmann::json;


namespace game {

    CombatSystem::CombatSystem() {
        json scene;
        std::ifstream file("..\\..\\data\\combat.json");
        file >> scene;
        file.close();

        for (auto& entity : scene["entities"]) {
            for (auto& character : Game::m_scene->getEntities()) {
                if (character.name == entity["name"].get<std::string>()) {
                    if (character.type == engine::EntityType::PLAYER)
                        hero = character;

                    Game::m_scene->registry().emplace<Combat>(
                            character.enttID,
                            entity["health"].get<unsigned int>(),
                            entity["damage"].get<unsigned int>(),
                            entity["armor"].get<unsigned int>()
                    );
                }
            }
        }
    }

    CombatSystem::~CombatSystem() = default;

    engine::Entity &CombatSystem::getHero() {
        return hero;
    }

    engine::Entity &CombatSystem::getEnemy() {
        return enemy;
    }

    void CombatSystem::update() {
        auto& movement = Game::m_scene->registry().get<engine::Movement>(hero.enttID);
        auto& positionHero = Game::m_scene->registry().get<engine::Transform>(hero.enttID).getPosition();
        auto& animationHero = Game::m_scene->registry().get<engine::AnimationInterface>(hero.enttID);

        if (Game::mousePicking->leftClickPressed()) {
            engine::Entity& entitySelected = Game::mousePicking->getEntityPicked();

            if (entitySelected.type != engine::EntityType::ENEMY) {
                auto& animationEnemy = Game::m_scene->registry().get<engine::AnimationInterface>(enemy.enttID);
                animationEnemy.currentAnimation = engine::Animation::idle;

                enemy = engine::Entity{};

                movement.direction = Game::mousePicking->getDirection();
                movement.moveTo = Game::mousePicking->getDirectionAugmented();
            } else {
               if (Game::m_scene->registry().get<engine::Status>(entitySelected.enttID).getType() == engine::Status::ACTIVE) {
                   enemy = entitySelected;

                   auto& positionEnemy = Game::m_scene->registry().get<engine::Transform>(enemy.enttID).getPosition();
                   float distanceX = positionEnemy.x - positionHero.x;
                   float distanceZ = positionEnemy.z - positionHero.z;

                   float radian = glm::atan(distanceZ, distanceX);
                   glm::vec3 moveTo = {positionEnemy.x - cos(radian) * 2, positionEnemy.y, positionEnemy.z - sin(radian) * 2};
                   movement.direction = glm::normalize(moveTo);
                   movement.moveTo = moveTo;
               }
            }
        }

        if (!enemy.name.empty()) {
            auto& positionEnemy = Game::m_scene->registry().get<engine::Transform>(enemy.enttID).getPosition();
            auto& animationEnemy = Game::m_scene->registry().get<engine::AnimationInterface>(enemy.enttID);
            auto& heroCombat = Game::m_scene->registry().get<Combat>(hero.enttID);
            auto& enemyCombat = Game::m_scene->registry().get<Combat>(enemy.enttID);
            float distanceX = positionEnemy.x - positionHero.x;
            float distanceZ = positionEnemy.z - positionHero.z;

            contact = (glm::sqrt(distanceX * distanceX + distanceZ * distanceZ) < 2.5f) && enemyCombat.isAlive;
            if (contact && !movement.isMoving) {
                animationEnemy.currentAnimation = engine::Animation::attack;
                animationHero.currentAnimation = engine::Animation::attack;
            }

            if (animationHero.reset && animationHero.currentAnimation == engine::Animation::attack && heroCombat.isAlive)
                calculateDamage(heroCombat, enemyCombat);

            if (enemyCombat.health == 0) {
                enemyCombat.isAlive = false;
                animationEnemy.currentAnimation = engine::Animation::death;
            }

            if (animationEnemy.reset && animationEnemy.currentAnimation == engine::Animation::attack && enemyCombat.isAlive)
                calculateDamage(enemyCombat, heroCombat);

            if (animationEnemy.reset && animationEnemy.currentAnimation == engine::Animation::death)
                animationEnemy.loop = false;
        } else {
            contact = false;
        }

        if (movement.isMoving && !contact)
            animationHero.currentAnimation = engine::Animation::idle;
    }

    void CombatSystem::setLuaBindings(sol::table game) {
        sol::table combatSystem = game["combatSystem"].get_or_create<sol::table>();
        combatSystem.set_function("getHero", &CombatSystem::getHero, this);
        combatSystem.set_function("getEnemy", &CombatSystem::getEnemy, this);
    }

    void CombatSystem::calculateDamage(Combat& obj1, Combat& obj2) {
        if (obj1.damage - obj2.armor <= obj2.health) {
            obj2.health -= obj1.damage - obj2.armor;
        } else {
            obj2.health = 0;
        }
    }

} // namespace game
