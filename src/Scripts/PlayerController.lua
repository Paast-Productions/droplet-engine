local trans = nil

local moveSpeed = 5.0

local mouseSensitivity = 0.0025   -- radians per pixel
local maxSine = 0.95   -- pitch limit up/down

local mouseLock = false

function Clamp(value, minValue, maxValue)
    return math.max(minValue, math.min(maxValue, value))
end

function ApplyMouseLook(dt)
    local dx, dy = 0.0, 0.0
    if (mouseLock) then
        dx = Input:GetDeltaMouseX()
        dy = Input:GetDeltaMouseY()
    end

    local yawDelta = -dx * mouseSensitivity
    local pitchDelta = dy * mouseSensitivity

    -- Rotate around y-axis
    trans:RotateAxis(yawDelta, Vec3.new(0.0, 1.0, 0.0), TransformSpace.World)
    
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
	trans = self:GetTransform()
    moveSpeed = 5.0
    
    tracy.ZoneEnd()
end

function OnUpdate(dt)
    tracy.ZoneBeginN("PlayerController OnUpdate")

    ApplyMouseLook(dt)
    if Input:KeyPressed(Key.Q) then
        if (mouseLock == false) then
            mouseLock = true
        else
            mouseLock = false
        end
    end

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