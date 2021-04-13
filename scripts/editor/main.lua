-- Editor UI Script
local window

function init()
    window = ui.createWindow("Test", 250.0, 100.0, ui.WindowFlags.reqOpen)
end

function drawData()

end

function drawUI()
    if window:open() then window:draw(drawData) end
end


