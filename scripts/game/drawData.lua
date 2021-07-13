-- Draw data
drawData = {}

function drawData.debugWindow()
    local mousePos = window.getCurrentMousePos()
    imgui.text("Mouse Position: x: "..tostring(mousePos.x).." y: "..tostring(mousePos.y))

    local entityPicked = mouse.getEntityPicked()
    imgui.text("Entity picked name: "..entityPicked.name)

    local origin = mouse.getOrigin()
    local direction = mouse.getDirection()
    local directionAugmented = mouse.getDirectionAugmented()
    imgui.text("Mouse origin: x: "..tostring(origin.x).." y: "..tostring(origin.y).." z: "..tostring(origin.z))
    imgui.text("Mouse direction: x: "..tostring(direction.x).." y: "..tostring(direction.y).." z: "..tostring(direction.z))
    imgui.text("Mouse direction augmented: x: "..tostring(directionAugmented.x).." y: "..tostring(directionAugmented.y).." z: "..tostring(directionAugmented.z))

    local transform = scene.components.getTransform(scene.entities:get(1).id)
    imgui.inputFloat3("Position", transform:getPosition())

    local angles = glm.degrees(transform:getRotation())
    imgui.inputFloat3("Rotation", angles)
    transform:setRotation(glm.radians(angles))
end
