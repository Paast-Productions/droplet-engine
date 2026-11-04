-- HACK: All commented code is not working in this script and is transferred to PlayerController.lua
-- The problem is that you can't modify the transform of a parent node

local trans = nil
-- local parent = nil
-- local parentTransform = nil

local mouseSensitivity = 0.0025   -- radians per pixel
local maxSine = 0.95   -- pitch limit up/down


function OnStart()
    tracy.ZoneBeginN("CameraController OnStart")

    print("New CameraController")
    if self:GetCameraComponent() == nil then
        print("There is no camera component on this node!")
    end
--     parent = self:GetParent()
--     if parent ~= nil then
--         parentTransform = parent:GetTransform()
--     end
	trans = self:GetTransform()
    
    tracy.ZoneEnd()
end

function OnUpdate(dt)
    tracy.ZoneBeginN("CameraController OnUpdate")
    
--     local dx = Input:GetDeltaMouseX()
    local dy = Input:GetDeltaMouseY()

--     local yawDelta = -dx * mouseSensitivity
    local pitchDelta = dy * mouseSensitivity
    
    -- Rotate around y-axis
--     if parent ~= nil then
--         print("Rotate")
--         parentTransform:RotateAxis(yawDelta, Vec3.new(0.0, 1.0, 0.0), TransformSpace.World)
--         
--         local pos = parentTransform:GetPosition(TransformSpace.World)
--         local sphere = Sphere.new(pos, 0.2)
--         local color = Vec4.new(1.0, 0.0, 0.0, 1.0)
--         DebugDraw.DrawSphere(sphere, color, 1, true)
--         
--     else
--         trans:RotateAxis(yawDelta, Vec3.new(0.0, 1.0, 0.0), TransformSpace.World)
--     end
    
    -- Rotate around local x-axis
    local forwardY = trans:GetForward().y
    if forwardY <= maxSine and pitchDelta < 0.0 then
        trans:RotateAxis(pitchDelta, trans:GetRight(), TransformSpace.Local)
    end
    if forwardY >= -maxSine and pitchDelta > 0.0  then
        trans:RotateAxis(pitchDelta, trans:GetRight(), TransformSpace.Local)
    end

    tracy.ZoneEnd()
end

function RenderUI()
    ImGui.Text("WARNING: This script only rotates the node around the local x-axis using the mouse.")
end
