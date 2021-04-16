require 'drawData'

-- Editor UI Script
openSaveScene = false
openLoadScene = false
openAddModel = false
openDemoWindow = false

-- Windows
local cameraControls
local entitiesPanel
local entityPropertiesPanel

menuBar = {
    [0] = {
       name = "File",
       items = {
           [0] = {
               name = "New",
               func = function()

               end
           },
           [1] = {
               name = "Save",
               func = function()
                    openSaveScene = not openSaveScene
               end
           },
           [2] = {
               name = "Open",
               func = function()
                    openLoadScene = not openLoadScene
               end
           }
       }
    },
    [1] = {
        name = "Entity",
        items = {
            [0] = {
                name = "Add Entity",
                func = function()
                    editor.addEntity("Object", "cube")
                end
            }
        }
    },
    [2] = {
        name = "Assets",
        items = {
            [0] = {
                name = "Add Models",
                func = function()
                    openAddModel = not openAddModel
                end
            }
        }
    },
    [3] = {
        name = "Tools",
        items = {
            [0] = {
                name = "Camera controls",
                func = function()
                    cameraControls:setState(not cameraControls:open());
                end
            },
            [1] = {
                name = "ImGui demo",
                func = function()
                    openDemoWindow = not openDemoWindow
                end
            }
        }
    }
}


function init()
    local windowSize = window.size()

    -- Camera scene controls(no editor object)
    cameraControls = ui.createWindow("Camera Controls", -1.0, -1.0, ui.WindowFlags.close)

    -- Entities panel
    entitiesPanel = ui.createWindow("Entities", 350.0, ((windowSize.height * 0.5) - 22), ui.WindowFlags.fixPosition)
    entitiesPanel:setPosition(0, 22);
    entitiesPanel:setState(true)

    -- Entity Properties Panel
    entityPropertiesPanel = ui.createWindow("Test Properties", 350.0, ((windowSize.height * 0.5)), ui.WindowFlags.fixPosition);
    entityPropertiesPanel:setPosition(0.0, windowSize.height * 0.5)
    entityPropertiesPanel:setState(true)
end

function drawUI()
    -- Windows
    if cameraControls:open() then cameraControls:draw(drawData.cameraControls, 0) end
    entitiesPanel:draw(drawData.entitiesPanel, imgui.flags.noCollapse | imgui.flags.noResize)
    entityPropertiesPanel:draw(drawData.entitiesPropertiesPanel, imgui.flags.noCollapse | imgui.flags.noResize)

    -- Functions
    if openSaveScene then editor.saveScene("Select directory and file name", ".json", "../data/", "openSaveScene") end
    if openLoadScene then editor.loadScene("Select a file", ".json", "../data/", "openLoadScene") end
    if openAddModel then editor.addModel("Choose File", ".gltf", "../Assets/models", "openAddModel") end
    if openDemoWindow then imgui.showDemo("openDemoWindow") end
end
