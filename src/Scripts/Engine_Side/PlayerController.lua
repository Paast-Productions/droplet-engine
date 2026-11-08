local trans = nil
local physCompHandle = nil

local mouseSensitivity = 0.0025   -- radians per pixel
local moveSpeed = 5.0

function OnStart()
    tracy.ZoneBeginN("PlayerController OnStart")

    print("New PlayerController")
	trans = self:GetTransform()
    
    tracy.ZoneEnd()
end

function OnUpdate(dt)
    tracy.ZoneBeginN("PlayerController OnUpdate")
    
    physCompHandle = self:GetPhysicsComponent()
    if physCompHandle == nil then
        return
    end
    
    -- HACK: This rotation code should've been in CameraController.lua but doesn't work there
    local dx = Input:GetDeltaMouseX()
    local yawDelta = -dx * mouseSensitivity
    trans:RotateAxis(yawDelta, Vec3.new(0.0, 1.0, 0.0), TransformSpace.World)

    local walkDir = Vec3.new(0.0, 0.0, 0.0)
	if Input:KeyHeld(Key.W) then
        walkDir = GLM.V3Add(walkDir, trans:GetForward())
    end

    if Input:KeyHeld(Key.A) then
        walkDir = GLM.V3Add(walkDir, trans:GetRight())
    end

    if Input:KeyHeld(Key.S) then
        walkDir = GLM.V3Sub(walkDir, trans:GetForward())
    end

    if Input:KeyHeld(Key.D) then
        walkDir = GLM.V3Sub(walkDir, trans:GetRight())
    end

    if walkDir.x ~= 0.0 or walkDir.y ~= 0.0 or walkDir.z ~= 0.0 then
        walkDir = GLM.V3MulF(GLM.V3Normalize(walkDir), moveSpeed)
    end
    walkDir.y = physCompHandle:GetVelocity().y
    physCompHandle:SetVelocity(walkDir)

    tracy.ZoneEnd()
end

function RenderUI()
    ImGui.Text("WARNING: This script rotates the node around the world y-axis using the mouse.")
end