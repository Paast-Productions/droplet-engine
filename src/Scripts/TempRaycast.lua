local cubeTrans = nil

function OnStart()
    cubeTrans = self:GetTransform()
end

function RenderUI()
    if cubeTrans == nil then return end

    local cubePos = cubeTrans:GetPosition(TransformSpace.World)
    ImGui.Text("Cube Position: " .. string.format("%.2f, %.2f, %.2f", cubePos.x, cubePos.y, cubePos.z))
end