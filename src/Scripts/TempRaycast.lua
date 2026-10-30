local cubeTrans = nil
local tablePos = Vec3.new(0, 0, 0)
local interactionRadius = 3.0 

function OnStart()
    print("temp raycast")
    cubeTrans = self:GetTransform()
end

function RenderUI()
    if cubeTrans == nil then return end

    local cubePos = cubeTrans:GetPosition(TransformSpace.World)
    ImGui.Text("Table Pos: " .. string.format("%.2f, %.2f, %.2f", cubePos.x, cubePos.y, cubePos.z))

    local vectorToTable = GLM.V3Sub(tablePos, cubePos)
    local distance = GLM.V3Length(vectorToTable)
    ImGui.Text("Table distance: " .. string.format("%.2f", distance))

    if distance <= interactionRadius then
        -- print("cube is near!")
        
        if Input:KeyPressed(Key.E) then
            print("intergrade with table!")
            cubeTrans = self:Destroy();
        end
    end
end