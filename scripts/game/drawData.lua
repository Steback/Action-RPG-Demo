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

local selectedIdle = true
local selectedAttack = false
local selectedDeath = false
local selectedWalk = false
local currentAnimationType = components.AnimationType.idle
function drawData.animationsControl()
    selectedIdle = imgui.checkBox("Idle", selectedIdle)
    if (selectedIdle) then
        selectedAttack = false
        selectedDeath = false
        selectedWalk = false
        currentAnimationType = components.AnimationType.idle
    end

    selectedAttack = imgui.checkBox("Attack", selectedAttack)
    if (selectedAttack) then
        selectedIdle = false
        selectedDeath = false
        selectedWalk = false
        currentAnimationType = components.AnimationType.attack
    end

    selectedDeath = imgui.checkBox("Death", selectedDeath)
    if (selectedDeath) then
        selectedIdle = false
        selectedAttack = false
        selectedWalk = false
        currentAnimationType = components.AnimationType.death
    end

    selectedWalk = imgui.checkBox("Walk", selectedWalk)
    if (selectedWalk) then
        selectedIdle = false
        selectedAttack = false
        selectedDeath = false
        currentAnimationType = components.AnimationType.walk
    end

    for i = 1, scene.entities:size() do
        local entity = scene.entities:get(i)

        if entity.components & components.type.animation then
            local animations = scene.components.getAnimation(entity.id)
            animations.currentType = currentAnimationType
        end
    end
end