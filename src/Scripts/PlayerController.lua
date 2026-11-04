local trans = nil
local parent = nil

local moveSpeed = 5.0

local mouseSensitivity = 0.0025   -- radians per pixel
local maxSine = 0.95   -- pitch limit up/down

local mouseLock = false

function ApplyMouseLook(dt)
    
    local dx = Input:GetDeltaMouseX()
    local dy = Input:GetDeltaMouseY()

    local yawDelta = -dx * mouseSensitivity
    local pitchDelta = dy * mouseSensitivity

    -- Rotate around y-axis
    if parent ~= nil then
        parent:GetTransform():RotateAxis(yawDelta, Vec3.new(0.0, 1.0, 0.0), TransformSpace.World)
    else
        trans:RotateAxis(yawDelta, Vec3.new(0.0, 1.0, 0.0), TransformSpace.World)
    end
    
    -- Rotate around local x-axis
    local forwardY = trans:GetForward().y
    if forwardY <= maxSine and pitchDelta < 0.0 then
        trans:RotateAxis(pitchDelta, trans:GetRight(), TransformSpace.World)
    end
    if forwardY >= -maxSine and pitchDelta > 0.0  then
        trans:RotateAxis(pitchDelta, trans:GetRight(), TransformSpace.World)
    end
end

function OnStart()
    tracy.ZoneBeginN("PlayerController OnStart")

    print("New PlayerController")
    
    parent = self:GetParent()
    trans = self:GetTransform()
    
    tracy.ZoneEnd()
end

function OnUpdate(dt)
    tracy.ZoneBeginN("PlayerController OnUpdate")

    ApplyMouseLook(dt)

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

    if walkDir.x ~= 0.0 or walkDir.z ~= 0.0 then
        walkDir.y = 0.0
        walkDir = GLM.V3MulF(GLM.V3MulF(GLM.V3Normalize(walkDir), moveSpeed), dt)
        trans:Move(walkDir, TransformSpace.World)
    end

    tracy.ZoneEnd()
end

function RenderUI()
    tracy.ZoneBeginN("PlayerController RenderUI")
    tracy.ZoneEnd()
end