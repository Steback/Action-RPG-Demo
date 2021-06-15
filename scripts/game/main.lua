require 'drawData'

local debugWindow
local animationsControl

function init()
    debugWindow = ui.createWindow("Debug Window", -1.0, -1.0, ui.WindowFlags.close)
    debugWindow:setState(true)

    animationsControl = ui.createWindow("Animations Control", 200, -1.0, 0)
    animationsControl:setState(true)
end

function drawUI()
    -- Windows
    debugWindow:draw(drawData.debugWindow, 0);
    animationsControl:draw(drawData.animationsControl, 0)

    -- Functions
end