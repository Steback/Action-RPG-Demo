-- Draw data
drawData = {}

function drawData.cameraControls()
    local camera = scene.getCamera()

    imgui.inputFloat3("Eye", camera:eye())
    imgui.inputFloat3("Center", camera:center())
    imgui.inputFloat3("Up", camera:up())
    imgui.separator()
    imgui.inputFloat2("Angles", camera:angles())
    imgui.inputFloat("FOV", camera:getFov())
    imgui.separator()
    imgui.inputFloat("Velocity", camera:getVelocity())
    imgui.inputFloat("Turn velocity", camera:getTurnVelocity())
    imgui.separator()
    imgui.inputFloat("Distance", camera:getDistance())
end


entitySelected = 0
function drawData.entitiesPanel()
    for i = 1, scene.entities:size() do
        local entity = scene.entities:get(i)

        if imgui.selectable(entity.name, entitySelected == i) then
            entitySelected = i
        end
    end

    editor.setEntity(entitySelected)
end

function drawData.entitiesPropertiesPanel()
    if (entitySelected ~= 0) then
        local entity = scene.entities:get(entitySelected);

        imgui.text("ID: "..entity.id)
        entity.name = imgui.inputText("Name", entity.name)

        imgui.text("Components: "..entity.flags)

        if (entity.flags & scene.EntityType.camera) ~= 0 then
            if imgui.collapsingHeader("Camera") then
                local camera = scene.components.getCamera(entity.id)

                imgui.inputFloat3("Eye", camera:eye())
                imgui.inputFloat3("Center", camera:center())
                imgui.separator()

                local angles = glm.degrees(camera:angles())
                imgui.inputFloat2("Angles", angles)
                camera:setAngles(glm.radians(angles))

                imgui.inputFloat("FOV", camera:getFov())
                imgui.separator()
                imgui.inputFloat("Velocity", camera:getVelocity())
                imgui.inputFloat("Turn velocity", camera:getTurnVelocity())
                imgui.separator()
                imgui.inputFloat("Distance", camera:getDistance())
            end
        else
            if imgui.collapsingHeader("Transform") then
                local transform = scene.components.getTransform(entity.id)

                imgui.inputFloat3("Position", transform:getPosition());

                local angles = glm.degrees(transform:getRotation())
                imgui.inputFloat3("Rotation", angles)
                transform:setRotation(glm.radians(angles))

                imgui.inputFloat3("Scale", transform:getScale())
                imgui.inputFloat("Velocity", transform:getVelocity(true))
            end
        end

        if entity.components & components.type.model then
            if imgui.collapsingHeader("Model") then
                local model = scene.components.getModel(entity.id)

                local modelsNames = editor.modelsNames
                local entitiesInfo = editor.entitiesInfo
                local entityInfo = entitiesInfo:get(entitySelected)
                local currentModel = entityInfo.model + 1

                imgui.beginCombo("Model name", modelsNames:get(currentModel), function()
                    for i = 1, modelsNames:size() do
                        if imgui.selectable(modelsNames:get(i), currentModel == i) then
                            currentModel = i;
                            entityInfo.model = currentModel - 1;

                            local modelID = tools.hashString(modelsNames:get(currentModel))
                            model:setModel(modelID)
                        end
                    end
                end)
            end
        end
    end
end