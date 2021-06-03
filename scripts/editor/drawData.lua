require 'utilities'

-- Draw data
drawData = {}

-- Secondary windows
local nodeView
local addComponentView

-- Main draw functions
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

        local entityTypes = {"Object", "Player", "Enemy", "Building", "Camera"}
        local currentType = entity.type + 1
        local currentTypeName = entityTypes[currentType]

        imgui.beginCombo("Type", currentTypeName, function()
            for i = 1, #entityTypes do
                if imgui.selectable(entityTypes[i], currentType == i) then
                    currentType = i
                    entity.type = currentType - 1
                end
            end
        end)

        if (entity.type == scene.EntityType.camera) then
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

        if (entity.components & components.type.model) then
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

                if imgui.button("Nodes") then
                    nodeView:setState(true)
                end
            end
        end


        if (entity.components & components.type.animation) ~= 0 then
            if imgui.collapsingHeader("Animations") then
                local animation = scene.components.getAnimation(entity.id)
                local animationsNames = editor.animationsNames
                local animationList = animation.animationsList
                local animationsType = {"Idle", "Attack", "Death", "Walk"}

                for i = 1, animationList:size() do
                    local currentAnimation = animationList:at(i)

                    imgui.beginCombo(animationsType[i], animationsNames:get(currentAnimation), function()
                        for key, name in animationsNames:pairs() do
                            if imgui.selectable(name, currentType == key) then
                                currentType = key
                                animationList:set(i, currentType)
                            end
                        end
                    end)
                end
            end
        end

        if imgui.button("Add Component") then
            addComponentView:setState(true)
        end
    end
end

-- Secondary draw functions
function nodeViewDraw()
    local model = scene.components.getModel(scene.entities:get(entitySelected).id)

    utilities.drawNode(model:getNode(model:getRootNode()), model)
end

function addComponent()
    if imgui.button("Animation") then
        editor.addComponent(scene.entities:get(entitySelected).id, components.type.animation)
        addComponentView:setState(false)
    end
end

-- Setup secondary windows
function drawData.setup()
    nodeView = ui.createWindow("Nodes view", -1.0, -1.0, ui.WindowFlags.close)
    addComponentView = ui.createWindow("Add Component", 200, -1.0, ui.WindowFlags.close)
end

-- Draw secondary windows
function drawData.draw()
    if nodeView:open() then nodeView:draw(nodeViewDraw, imgui.flags.noCollapse | imgui.flags.noResize) end
    if addComponentView:open() then addComponentView:draw(addComponent, imgui.flags.noCollapse | imgui.flags.noResize) end
end
