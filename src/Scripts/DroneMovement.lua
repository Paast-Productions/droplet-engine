-- DroneMovement.lua

--
-- Member Variables
--

-- Movement settings
local acceleration = 5.0
local maxSpeed = 8.0
local drag = 1.5

-- Runtime state
local trans = nil
local velocity = nil


-- fih.lua variables
local orbitPoint = Vec3.new(0.0, 0.0, 0.0)
local targetPoint = Vec3.new(0.0, 0.0, 0.0)
local targetRadius = 3.0
local targetAcceptanceRadius = 0.2

local maxMoveSpeed = 3.0
local maxTurnSpeed = 1.0

local moveSpeedRateOfChange = 0.3
local turnSpeedRateOfChange = 0.8

local moveSpeed = 0.0
local turnSpeedX = 0.0 -- Turn speed around global Y axis (yaw)
local turnSpeedY = 0.0 -- Turn speed around local X axis (pitch)


--
-- Member Functions fih.lua
--
function Clamp(value, minValue, maxValue)
    return math.max(minValue, math.min(maxValue, value))
end

function MoveTowards(current, target, maxDelta)
    if current < target then
        return math.min(current + maxDelta, target)
    else
        return math.max(current - maxDelta, target)
    end
end

function WrapAngle(angle)
    while angle > math.pi do
        angle = angle - 2.0 * math.pi
    end

    while angle < -math.pi do
        angle = angle + 2.0 * math.pi
    end

    return angle
end

function NewTargetPoint()
    tracy.ZoneBeginN("NewTargetPoint")
    
	-- Random point on a sphere using spherical coordinates
	local theta = math.random() * 2 * math.pi
    local phi = math.acos(2 * math.random() - 1)

    -- Random distance, uniform by volume
    local r = targetRadius * math.random()^(1/3)

	local v = Vec3.new(0.0, 0.0, 0.0)
    v.x = r * math.sin(phi) * math.cos(theta)
    v.y = r * math.sin(phi) * math.sin(theta)
    v.z = r * math.cos(phi)

	targetPoint = GLM.V3Add(orbitPoint, v)

    tracy.ZoneEnd()
end

function IsWithinTarget()
    tracy.ZoneBeginN("IsWithinTarget")

	local pos = trans:GetPosition(TransformSpace.World)
	local dist = GLM.V3Distance(pos, targetPoint)
    
    tracy.ZoneEnd()
	return (dist <= targetAcceptanceRadius)
end

function UpdateVelocity(dt)
    tracy.ZoneBeginN("UpdateVelocity")

	-- TODO: Steering behavior to move forwards and turn towards target point
	 local pos = trans:GetPosition(TransformSpace.World)

    -- Direction towards the target
    local toTarget = GLM.V3Sub(targetPoint, pos)
    local distance = GLM.V3Length(toTarget)

    if distance < 0.0001 then
        moveSpeed = MoveTowards(
            moveSpeed, 0.0, moveSpeedRateOfChange * dt
        )
        turnSpeedX = MoveTowards(
            turnSpeedX, 0.0, turnSpeedRateOfChange * dt
        )
        turnSpeedY = MoveTowards(
            turnSpeedY, 0.0, turnSpeedRateOfChange * dt
        )

        tracy.ZoneEnd()
        return
    end

    toTarget = GLM.V3MulF(toTarget, 1.0 / distance)

    local forward = trans:GetForward()

    -- Calculate yaw error around global Y.
    local currentYaw = math.atan(forward.x, forward.z)
    local targetYaw = math.atan(toTarget.x, toTarget.z)
    local yawError = WrapAngle(targetYaw - currentYaw)

    -- Calculate pitch error.
    local currentPitch = math.atan(
        forward.y,
        math.sqrt(forward.x * forward.x + forward.z * forward.z)
    )

    local targetPitch = math.atan(
        toTarget.y,
        math.sqrt(toTarget.x * toTarget.x + toTarget.z * toTarget.z)
    )

    local pitchError = targetPitch - currentPitch

    -- Convert angular errors to desired turn speeds.
    local turnGain = 2.0

    local desiredTurnX = Clamp(
        yawError * turnGain,
        -maxTurnSpeed,
        maxTurnSpeed
    )

    local desiredTurnY = Clamp(
        pitchError * turnGain,
        -maxTurnSpeed,
        maxTurnSpeed
    )

    -- Smoothly change turn speeds.
    turnSpeedX = MoveTowards(
        turnSpeedX,
        desiredTurnX,
        turnSpeedRateOfChange * dt
    )

    turnSpeedY = MoveTowards(
        turnSpeedY,
        desiredTurnY,
        turnSpeedRateOfChange * dt
    )

    -- Move forward, slowing down when misaligned or close to target.
    local yawAlignment = math.cos(yawError)
    local pitchAlignment = math.cos(pitchError)

    local alignment = Clamp(
        yawAlignment * pitchAlignment,
        0.0,
        1.0
    )

    -- Slow down as the fish approaches the target,
	-- but keep moving until it enters the acceptance radius.
	local brakingDistance = math.max(targetAcceptanceRadius * 2.0, 0.5)

	local distanceFactor = Clamp(
		(distance - targetAcceptanceRadius) / brakingDistance,
		0.0,
		1.0
	)

	-- Maintain some forward speed while outside the target radius.
	local minSpeedFactor = 0.2

	local desiredMoveSpeed = maxMoveSpeed *
		math.max(minSpeedFactor, distanceFactor) *
		alignment

	-- Only stop when actually within the acceptance radius.
	if distance <= targetAcceptanceRadius then
		desiredMoveSpeed = 0.0
	end

	moveSpeed = MoveTowards(
		moveSpeed,
		desiredMoveSpeed,
		moveSpeedRateOfChange * dt
	)
    
    tracy.ZoneEnd()
end

function ApplyVelocity(dt)
    tracy.ZoneBeginN("ApplyVelocity")

	local toMove = GLM.V3MulF(GLM.V3MulF(trans:GetForward(), moveSpeed), dt)
	trans:Move(toMove, TransformSpace.World)

	local toRotateY = GLM.AngleAxis(trans:GetRight(), turnSpeedY * dt)
	trans:Rotate(toRotateY)

	local toRotateX = GLM.AngleAxis(Vec3.new(0.0, 1.0, 0.0), turnSpeedX * dt)
	trans:Rotate(toRotateX, TransformSpace.World)

    tracy.ZoneEnd()
end


--
-- Member functions
--

function UpdateMovement(dt)
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
    if Input:KeyHeld(Key.D) then
        inputDirection = GLM.V3Add(
            inputDirection,
            trans:GetRight(TransformSpace.World)
        )
    end

    if Input:KeyHeld(Key.A) then
        inputDirection = GLM.V3Sub(
            inputDirection,
            trans:GetRight(TransformSpace.World)
        )
    end

    -- Dedicated vertical movement in world space
    local worldUp = Vec3.new(0.0, 0.0, 0.0)

    if Input:KeyHeld(Key.Space) then
        inputDirection = GLM.V3Add(inputDirection, worldUp)
    end

    if Input:KeyHeld(Key.LCTRL) then
        inputDirection = GLM.V3Sub(inputDirection, worldUp)
    end

    -- Normalize only if the direction is longer than 1
    -- This prevents diagonal movement from increasing speed
    local inputLength = GLM.V3Length(inputDirection)

    if inputLength > 1.0 then
        inputDirection = GLM.V3Normalize(inputDirection)
    end

    -- Accelerate while input is held
    if inputLength > 0.0 then 
        local accelerationStep = GLM.V3MulF(
            inputDirection,
            acceleration * dt
        )

        velocity = GLM.V3Add(velocity, accelerationStep)
    else
        local dragFactor = math.max(0.0, 1.0 - drag * dt)

        velocity = GLM.V3MulF(velocity, dragFactor)
    end

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
end



--
-- Auto Functions
--

function OnStart()
    tracy.ZoneBeginN("Drone_movement OnStart")

    print("New Drone_movement")
    local test = Vec3.new(0.0, 0.0, 0.0)
    print(test.x)
	trans = self:GetTransform()
    velocity = Vec3.new(0.0, 0.0, 0.0)
    
    tracy.ZoneEnd()
end

function OnUpdate(dt)
    tracy.ZoneBeginN("Drone_movement OnUpdate")

	UpdateMovement(dt)

    tracy.ZoneEnd()
end

function RenderUI()
    tracy.ZoneBeginN("Drone_movement RenderUI")

    if ImGui.Begin("Drone Movement") then
        local changed, newAcceleration =
            ImGui.SliderFloat(
                "Acceleration",
                acceleration,
                0.5,
                20.0
            )

        if changed then
            acceleration = newAcceleration
        end

        local speedChanged, newMaxSpeed =
            ImGui.SliderFloat(
                "Maximum Speed",
                maxSpeed,
                1.0,
                30.0
            )

        if speedChanged then
            maxSpeed = newMaxSpeed
        end

        local dragChanged, newDrag =
            ImGui.SliderFloat(
                "Drag",
                drag,
                0.0,
                5.0
            )

        if dragChanged then
            drag = newDrag
        end

        ImGui.Separator()
        ImGui.Text("W/S: Forward / Backward")
        ImGui.Text("A/D: Strafe Left / Right")
        ImGui.Text("Space / Left Ctrl: Up / Down")
    end

    ImGui.End()

    tracy.ZoneEnd()
end
