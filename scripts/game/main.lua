require 'drawData'

local healthHero
local healthEnemy


function init()
    local barWidth = 350
    local barHeight = 60

    healthHero = ui.createWindow("Hero", barWidth, barHeight, 0)
    healthHero:setState(true)

    healthEnemy = ui.createWindow("Enemy", barWidth, barHeight, 0)
    healthEnemy:setState(true)
end

function drawUI()
    -- Windows
    healthHero:draw(drawData.healthHero, imgui.flags.noCollapse | imgui.flags.noResize);
    healthEnemy:draw(drawData.healthEnemy, imgui.flags.noCollapse | imgui.flags.noResize);

    -- Functions
end