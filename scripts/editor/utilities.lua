-- Tools
utilities = {}

function utilities.drawNode(node, model)
    imgui.treeNode(node.name, function()
        imgui.text("ID: "..tostring(node.id))

        imgui.separator()
        local position = node.position
        imgui.text("Position  x: "..tostring(position.x).."  y: "..tostring(position.y).."  z: "..tostring(position.z))

        imgui.separator()
        local rotation = node.rotation
        imgui.text("Rotation  x: "..tostring(rotation.x).."  y: "..tostring(rotation.y).."  z: "..tostring(rotation.z).."  w: "..tostring(rotation.w))

        imgui.separator()
        local scale = node.scale
        imgui.text("Scale  x: "..tostring(scale.x).."  y: "..tostring(scale.y).."  z: "..tostring(scale.z))

        imgui.separator()
        imgui.text("Mesh ID: "..tostring(node.mesh))

        imgui.separator()
        imgui.text("Parent ID: "..tostring(node.parent))

        imgui.separator()
        if not node.children:empty() then
            imgui.text("Children: ")

            for i = 1, node.children:size() do
                utilities.drawNode(model:getNode(node.children:get(i)), model)
            end
        end
    end)
end