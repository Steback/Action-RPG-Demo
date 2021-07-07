require 'drawData'

local debugWindow

function init()
    debugWindow = ui.createWindow("Debug Window", -1.0, -1.0, ui.WindowFlags.close)
    debugWindow:setState(true)
end

function drawUI()
    -- Windows
    debugWindow:draw(drawData.debugWindow, 0);

    -- Functions
end