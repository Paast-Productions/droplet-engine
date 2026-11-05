
--
-- Member Variables
--

local trans = nil
local velocity = nil

-- Movement settings
local acceleration = 5.0
local maxSpeed = 8.0

local acceleration = 5.0     -- speeding up along the input direction
local deceleration = 12.0    -- no input: slowing to a stop
local braking = 14.0         -- input opposes current motion: turning around

local moveSpeed = 0.0


local mouseSensitivity = 0.0025   -- radians per pixel
local maxPitch = math.rad(80.0)   -- pitch limit up/down
local invertY = false

local mouseSmoothTime = 0.04   -- seconds, 0 = raw input
local smoothDX = 0.0
local smoothDY = 0.0

local mouseLock = false

--
-- Member Functions
--
function Clamp(value, minValue, maxValue)
    return math.max(minValue, math.min(maxValue, value))
end

-- Move a vector toward a target by at most maxDelta (vector version of MoveTowards)
function MoveTowardsV3(current, target, maxDelta)
    local delta = GLM.V3Sub(target, current)
    local dist = GLM.V3Length(delta)

    if dist <= maxDelta or dist < 0.00001 then
        return target
    end

    return GLM.V3Add(current, GLM.V3MulF(delta, maxDelta / dist))
end

function ApplyMouseLook(dt)
    local dx, dy = 0.0, 0.0
    if (mouseLock) then
        dx = Input:GetDeltaMouseX()
        dy = Input:GetDeltaMouseY()
    end
    -- Frame-rate independent one-pole filter. Total rotation is preserved,
    -- it's just spread over a few frames.
    local a = 1.0
    if mouseSmoothTime > 0.0001 then
        a = 1.0 - math.exp(-dt / mouseSmoothTime)
    end
    smoothDX = smoothDX + (dx - smoothDX) * a
    smoothDY = smoothDY + (dy - smoothDY) * a

    -- Keep running until the tail has decayed, then stop
    if math.abs(smoothDX) < 0.001 and math.abs(smoothDY) < 0.001 then
        smoothDX, smoothDY = 0.0, 0.0
        return
    end

    local yawDelta = -smoothDX * mouseSensitivity
    local pitchDelta = -smoothDY * mouseSensitivity
    if invertY then pitchDelta = -pitchDelta end

    local forward = trans:GetForward()
    local currentPitch = math.atan(
        forward.y,
        math.sqrt(forward.x * forward.x + forward.z * forward.z)
    )
    local newPitch = Clamp(currentPitch + pitchDelta, -maxPitch, maxPitch)
    pitchDelta = newPitch - currentPitch

    trans:Rotate(GLM.AngleAxis(trans:GetRight(), -pitchDelta))
    trans:Rotate(GLM.AngleAxis(Vec3.new(0.0, 1.0, 0.0), yawDelta), TransformSpace.World)
end

function UpdateMovement(dt)
    tracy.ZoneBeginN("Drone_movement UpdateMovement")
    -- Build the movement input directions
    local inputDirection = Vec3.new(0.0, 0.0, 0.0)

    -- Forward and backward
    if Input:KeyHeld(Key.W) then
        inputDirection = GLM.V3Add(
            inputDirection,
            trans:GetForward(TransformSpace.World)
        )
    end

    if Input:KeyHeld(Key.S) then
        inputDirection = GLM.V3Sub(
            inputDirection,
            trans:GetForward(TransformSpace.World)
        )
    end

    -- Strafe left and right
    if Input:KeyHeld(Key.A) then
        inputDirection = GLM.V3Add(
            inputDirection,
            trans:GetRight(TransformSpace.World)
        )
    end

    if Input:KeyHeld(Key.D) then
        inputDirection = GLM.V3Sub(
            inputDirection,
            trans:GetRight(TransformSpace.World)
        )
    end

    -- Dedicated vertical movement in world space

    if Input:KeyHeld(Key.Space) then
        inputDirection = GLM.V3Add(
            inputDirection,
            trans:GetUp(TransformSpace.World)
        )
    end

    if Input:KeyHeld(Key.LCTRL) then
        inputDirection = GLM.V3Sub(
            inputDirection,
            trans:GetUp(TransformSpace.World)
        )
    end

       -- Normalize only if the direction is longer than 1
    local inputLength = GLM.V3Length(inputDirection)

    if inputLength > 1.0 then
        inputDirection = GLM.V3Normalize(inputDirection)
        inputLength = 1.0
    end

    -- Target velocity is what the player is asking for
    local targetVelocity = GLM.V3MulF(inputDirection, maxSpeed)

    -- Pick the rate depending on the situation
    local rate = deceleration                      -- no input: stop

    if inputLength > 0.0 then
        local dot = velocity.x * inputDirection.x
                  + velocity.y * inputDirection.y
                  + velocity.z * inputDirection.z

        if dot < 0.0 then
            rate = braking                         -- pushing against motion
        else
            rate = acceleration                    -- pushing with motion
        end
    end

    velocity = MoveTowardsV3(velocity, targetVelocity, rate * dt)

    -- Limit the total velocity, regardless of direction
    local speed = GLM.V3Length(velocity)

    if speed > maxSpeed then
        velocity = GLM.V3MulF(
            GLM.V3Normalize(velocity),
            maxSpeed
        )
    end

    -- Apply movement in world space
    local movement = GLM.V3MulF(velocity, dt)

    trans:Move(movement, TransformSpace.World)

    tracy.ZoneEnd()
end
--
-- Auto Functions
--

function OnStart()
    tracy.ZoneBeginN("Drone_movement OnStart")

    print("New Drone_movement")
	trans = self:GetTransform()
    moveSpeed = 2.5
    mouseSmoothTime = 0.050
    velocity = Vec3.new(0.0, 0.0, 0.0)
    tracy.ZoneEnd()
end

function OnUpdate(dt)
    tracy.ZoneBeginN("Drone_movement OnUpdate")

    ApplyMouseLook(dt)
    if Input:KeyPressed(Key.Tab) then
        if (mouseLock == false) then
            mouseLock = true
        else
            mouseLock = false
        end
    end
	
    UpdateMovement(dt)

    tracy.ZoneEnd()
end

function RenderUI()
    tracy.ZoneBeginN("Drone_movement RenderUI")

	ImGui.Spacing()
    _, mouseSmoothTime = ImGui.DragFloat("Mouse Smoothing (s)", mouseSmoothTime, 0.005, 0.0, 0.3)
    _, acceleration = ImGui.SliderFloat("Acceleration", acceleration, 0.5, 20.0)
    _, maxSpeed = ImGui.SliderFloat("Maximum Speed", maxSpeed, 1.0, 30.0)
    _, deceleration = ImGui.SliderFloat("Deceleration", deceleration, 0.5, 40.0)
    _, braking = ImGui.SliderFloat("Braking", braking, 0.5, 40.0)
	ImGui.Text("Current Move Speed: " .. moveSpeed)

    tracy.ZoneEnd()
end
