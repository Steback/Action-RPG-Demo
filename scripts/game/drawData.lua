local HERO_MAX_HEALTH = 100
local ENEMY_MAX_HEALTH = 50

-- Draw data
drawData = {}

function drawData.healthHero()
    local combat = game.getCombatComponent(game.combatSystem.getHero().id)
    imgui.progressBarBuf(combat.health / HERO_MAX_HEALTH, -1.0, -1.0, HERO_MAX_HEALTH);
end

function drawData.healthEnemy()
    local enemy = game.combatSystem.getEnemy()

    if enemy.name ~= "" then
        local combat = game.getCombatComponent(enemy.id)
        imgui.progressBarBuf(combat.health / ENEMY_MAX_HEALTH, -1.0, -1.0, ENEMY_MAX_HEALTH);
    end
end
