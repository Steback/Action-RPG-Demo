-- Draw data
drawData = {}

function drawData.debugWindow()
    local mousePos = window.getCurrentMousePos()
    imgui.text("Mouse Position: x: "..tostring(mousePos.x).." y: "..tostring(mousePos.y))

    local entityPicked = mouse.getEntityPicked()
    imgui.text("Entity picked name: "..entityPicked.name)

    local transform = scene.components.getTransform(scene.entities:get(1).id)
    imgui.inputFloat3("Position", transform:getPosition())
end
