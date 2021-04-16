-- Draw data
drawData = {}

function defaultFormat(num)
    return string.format("%.6f", num)
end

function drawData.debugWindow()
    local frameRate = imgui.getFrameRate()
    local deltaTime = tools.getDeltaTime()

    imgui.text("Delta time: "..defaultFormat(deltaTime))
    imgui.text("1 / 60: "..defaultFormat(1 / 60.0))
    imgui.text("Application average "..defaultFormat(1000.0 / frameRate).." ms/frame ("..defaultFormat(frameRate).." FPS)")
end