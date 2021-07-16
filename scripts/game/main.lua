require 'drawData'

local healthHero
local healthEnemy


function init()
    local barWidth = 350
    local barHeight = 60

    healthHero = ui.createWindow("Hero", barWidth, barHeight, ui.WindowFlags.fixPosition)
    healthHero:setPosition(60, 60);
    healthHero:setState(true)

    healthEnemy = ui.createWindow("Enemy", barWidth, barHeight, ui.WindowFlags.fixPosition)
    healthEnemy:setPosition(450, 60);
    healthEnemy:setState(true)
end

function drawUI()
    -- Windows
    healthHero:draw(drawData.healthHero, imgui.flags.noCollapse | imgui.flags.noResize);
    healthEnemy:draw(drawData.healthEnemy, imgui.flags.noCollapse | imgui.flags.noResize);

    -- Functions
end