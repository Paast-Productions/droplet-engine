--
-- Member Variables
--
local WORLD_UP = Vec3.new(0.0, 1.0, 0.0)

local trans = nil
local velocity = nil          -- base (player-controlled) velocity
local boostVelocity = nil     -- pulse velocity, decays on its own curve
local physicsComponent = nil

-- Keys (change these in ONE place if your engine names them differently)
local keyForward = Key.W
local keyBack    = Key.S
local keyLeft    = Key.A
local keyRight   = Key.D
local keyUp      = Key.Space   -- was Key.SPACE in one place, Key.Space in another
local keyDown    = Key.LCTRL   -- was Key.CTRL in one place, Key.LCTRL in another

-- Movement settings
local maxSpeed = 10.0
local acceleration = 5.0     -- speeding up along the input direction
local deceleration = 4.0     -- no input: slowing to a stop
local braking = 8.0          -- input opposes current motion: turning around

local moveSpeed = 0.0

-- Pulse jet settings
local pulseKey = Key.SHIFT
local minPulseCooldown = 1.0         -- cooldown after a tap
local maxPulseCooldown = 3.0         -- cooldown after a full charge
local currentPulseCooldown = 0.0
local pulseCooldownTotal = 1.0       -- for UI

local pulseCharging = false
local pulseChargeTime = 0.0
local maxPulseChargeTime = 1.0
local pulseChargeMovementMultiplier = 0.2

local minPulseImpulse = 20.0         -- tap
local maxPulseImpulse = 80.0         -- full charge
local boostDrag = 3.5                -- higher = shorter glide
local pulseRedirect = 0.4            -- how much old velocity survives a pulse (0..1)

local pulseDirection = WORLD_UP
local hasLatchedDirection = false

-- Camera FOV effects
local cameraComponent = nil
local camera = nil
local defaultFov = 90.0
local fovInitialised = false

local pulseChargeFov = -5.0          -- FOV offset at full charge (negative = pull in, positive = widen)
local chargeFovSmoothing = 10.0      -- how quickly the charge offset follows its target
local minFovKick = 6.0               -- FOV punch on a tap release
local maxFovKick = 16.0              -- FOV punch on a full-charge release
local fovKickDecay = 8.0             -- higher = shorter punch

local chargeFovOffset = 0.0
local fovKick = 0.0

-- Pulse sound
local AudioPlayer = nil

-- Camera settings
local mouseSensitivity = 0.005   -- radians per pixel
local maxPitch = math.rad(80.0)      -- pitch limit up/down
local invertY = false

local chargeSensMultiplier = 0.6     -- mouse sensitivity while charging
local sensBlendSpeed = 12.0          -- how quickly sensitivity eases in/out
local sensMultiplier = 1.0

local mouseSmoothTime = 0.04         -- seconds, 0 = raw input
local smoothDX = 0.0
local smoothDY = 0.0

local MovementModifiers = require("MovementModifiers")
--
-- Member Functions
--
function Clamp(value, minValue, maxValue)
    return math.max(minValue, math.min(maxValue, value))
end

function Lerp(a, b, t)
    return a + (b - a) * t
end

function SmoothStep(t)
    t = Clamp(t, 0.0, 1.0)
    return t * t * (3.0 - 2.0 * t)
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

-- Fetches the camera once and remembers the base FOV
function EnsureCamera()
    if camera ~= nil then
        return true
    end

    cameraComponent = self:GetCameraComponent()
    if cameraComponent == nil then
        return false
    end

    camera = cameraComponent:GetCamera()
    if camera == nil then
        return false
    end

    if not fovInitialised then
        defaultFov = camera:GetFOV()
        fovInitialised = true
    end
    return true
end

-- Raw (un-normalised) movement input in world space. Shared by movement and pulse aiming.
function GetInputDirection()
    local direction = Vec3.new(0.0, 0.0, 0.0)

    if Input:KeyHeld(keyForward) then
        direction = GLM.V3Add(direction, trans:GetForward(TransformSpace.World))
    end
    if Input:KeyHeld(keyBack) then
        direction = GLM.V3Sub(direction, trans:GetForward(TransformSpace.World))
    end
    if Input:KeyHeld(keyLeft) then
        direction = GLM.V3Add(direction, trans:GetRight(TransformSpace.World))
    end
    if Input:KeyHeld(keyRight) then
        direction = GLM.V3Sub(direction, trans:GetRight(TransformSpace.World))
    end

    -- Vertical movement in world space (independent of drone rotation)
    if Input:KeyHeld(keyUp) then
        direction = GLM.V3Add(direction, WORLD_UP)
    end
    if Input:KeyHeld(keyDown) then
        direction = GLM.V3Sub(direction, WORLD_UP)
    end

    return direction
end

function ApplyMouseLook(dt)
    if not EnsureCamera() then
        print("No CameraComponent found in DroneControl.lua")
        return
    end

    -- Ease the sensitivity toward the charge/non-charge target.
    local targetSens = 1.0
    if pulseCharging then
        targetSens = chargeSensMultiplier
    end
    sensMultiplier = sensMultiplier + (targetSens - sensMultiplier) * (1.0 - math.exp(-sensBlendSpeed * dt))

    local dx = Input:GetDeltaMouseX()
    local dy = Input:GetDeltaMouseY()

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

    local sens = mouseSensitivity * sensMultiplier
    local yawDelta = -smoothDX * sens
    local pitchDelta = -smoothDY * sens
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

-- Fires the pulse. Called on the frame the key is released while charging.
function ReleasePulse()
    local chargeT = SmoothStep(pulseChargeTime / maxPulseChargeTime)

    -- Kill most of the old momentum so the pulse goes where you aimed,
    -- then launch the boost as its own velocity.
    local impulse = Lerp(minPulseImpulse, maxPulseImpulse, chargeT)
    velocity = GLM.V3MulF(velocity, pulseRedirect)
    boostVelocity = GLM.V3MulF(pulseDirection, impulse)

    -- Bigger charge = longer cooldown
    pulseCooldownTotal = Lerp(minPulseCooldown, maxPulseCooldown, chargeT)
    currentPulseCooldown = pulseCooldownTotal

    -- FOV punch on release
    fovKick = Lerp(minFovKick, maxFovKick, chargeT)

    pulseCharging = false
    pulseChargeTime = 0.0
    hasLatchedDirection = false

    if AudioPlayer ~= nil and not AudioPlayer:IsPlaying() then
        AudioPlayer:Play()
    end
end

function UpdatePulseJet(dt)
    local pulseHeld = Input:KeyHeld(pulseKey)

    -- Cooldown
    if currentPulseCooldown > 0.0 then
        currentPulseCooldown = math.max(0.0, currentPulseCooldown - dt)
    end

    -- Decay the boost on its own curve (frame-rate independent)
    boostVelocity = GLM.V3MulF(boostVelocity, math.exp(-boostDrag * dt))
    if GLM.V3Length(boostVelocity) < 0.05 then
        boostVelocity = Vec3.new(0.0, 0.0, 0.0)
    end

    -- Start charging
    if pulseHeld and not pulseCharging and currentPulseCooldown <= 0.0 then
        pulseCharging = true
        pulseChargeTime = 0.0
        hasLatchedDirection = false
        pulseDirection = trans:GetForward(TransformSpace.World)
    end

    if pulseCharging then
        pulseChargeTime = math.min(pulseChargeTime + dt, maxPulseChargeTime)

        -- Latch the most recent non-zero input direction so letting go of a key
        -- a few ms early doesn't change where the pulse goes. With no input at all,
        -- aim along the camera.
        local input = GetInputDirection()
        if GLM.V3Length(input) > 0.001 then
            pulseDirection = GLM.V3Normalize(input)
            hasLatchedDirection = true
        elseif not hasLatchedDirection then
            pulseDirection = trans:GetForward(TransformSpace.World)
        end

        if not pulseHeld then
            ReleasePulse()
        end
    end
end

-- FOV is set in one place, every frame, so it can never get stuck.
function UpdateCamera(dt)
    if not EnsureCamera() then
        return
    end

    local targetOffset = 0.0
    if pulseCharging then
        targetOffset = pulseChargeFov * (pulseChargeTime / maxPulseChargeTime)
    end
    chargeFovOffset = chargeFovOffset + (targetOffset - chargeFovOffset) * (1.0 - math.exp(-chargeFovSmoothing * dt))

    fovKick = fovKick * math.exp(-fovKickDecay * dt)
    if math.abs(fovKick) < 0.01 then
        fovKick = 0.0
    end

    camera:SetFOV(defaultFov + chargeFovOffset + fovKick)
end

function UpdateMovement(dt)
    tracy.ZoneBeginN("DroneControl UpdateMovement")

    UpdatePulseJet(dt)

    if physicsComponent == nil then
        physicsComponent = self:GetPhysicsComponent()
        if physicsComponent == nil then
            print("No PhysicsComponent found in DroneControl.lua")
            tracy.ZoneEnd()
            return
        end
    end

    local chargeMovementMultiplier = 1.0
    if pulseCharging then
        chargeMovementMultiplier = pulseChargeMovementMultiplier
    end

    local inputDirection = GetInputDirection()

    -- Get movement modifiers.
    local speedModifier = MovementModifiers.GetSpeedMultiplier()
    local handlingModifier = MovementModifiers.GetHandling()

    -- The charge multiplier only affects the TARGET velocity and acceleration,
    -- not the hard speed clamp, so starting a charge eases you down instead of
    -- snapping your speed.
    local effectiveMaxSpeed = maxSpeed * speedModifier
    local effectiveAcceleration = acceleration * speedModifier * chargeMovementMultiplier
    local effectiveDeceleration = deceleration * handlingModifier
    local effectiveBraking = braking * handlingModifier

    -- Normalize only if the direction is longer than 1
    -- This prevents diagonal movement from increasing speed
    local inputLength = GLM.V3Length(inputDirection)
    if inputLength > 1.0 then
        inputDirection = GLM.V3Normalize(inputDirection)
        inputLength = 1.0
    end

    local targetVelocity = GLM.V3MulF(inputDirection, effectiveMaxSpeed * chargeMovementMultiplier)
    local targetSpeed = GLM.V3Length(targetVelocity)
    local currentSpeed = GLM.V3Length(velocity)

    -- Pick the rate depending on the situation.
    local rate = effectiveDeceleration

    if inputLength > 0.0 then
        local dot = velocity.x * inputDirection.x
                  + velocity.y * inputDirection.y
                  + velocity.z * inputDirection.z

        if dot < 0.0 then
            rate = effectiveBraking
        else
            rate = effectiveAcceleration
        end
    end

    -- Charging while faster than the reduced target: ease down at the normal
    -- deceleration rate rather than the (slowed) charge acceleration.
    if pulseCharging and currentSpeed > targetSpeed then
        rate = math.max(rate, effectiveDeceleration)
    end

    velocity = MoveTowardsV3(velocity, targetVelocity, rate * dt)

    -- Hard cap on base movement only. The pulse lives in boostVelocity,
    -- so it never needs special-casing here.
    local speed = GLM.V3Length(velocity)
    if speed > effectiveMaxSpeed then
        velocity = GLM.V3MulF(GLM.V3Normalize(velocity), effectiveMaxSpeed)
    end

    physicsComponent:SetVelocity(GLM.V3Add(velocity, boostVelocity))

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
    boostVelocity = Vec3.new(0.0, 0.0, 0.0)

    if not EnsureCamera() then
        print("No CameraComponent found in DroneControl.lua")
    end

    physicsComponent = self:GetPhysicsComponent()
    if physicsComponent == nil then
        print("No PhysicsComponent found in DroneControl.lua")
    end

    AudioPlayer = self:GetAudioPlayer("pulse-jet")
    if AudioPlayer == nil then
        print("Unable to find pulse-jet sound (DroneControl.lua)")
    end

    tracy.ZoneEnd()
end

function OnUpdate(dt)
    tracy.ZoneBeginN("Drone_movement OnUpdate")

    if trans == nil then
        trans = self:GetTransform()
        if trans == nil then
            print("No trans found in DroneControl.lua")
            tracy.ZoneEnd()
            return
        end
    end

    if physicsComponent == nil then
        physicsComponent = self:GetPhysicsComponent()
        if physicsComponent == nil then
            print("No PhysicsComponent found in DroneControl.lua")
            tracy.ZoneEnd()
            return
        end
    end

    if not EnsureCamera() then
        print("No CameraComponent found in DroneControl.lua")
        tracy.ZoneEnd()
        return
    end

    ApplyMouseLook(dt)
    UpdateMovement(dt)
    UpdateCamera(dt)

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

    ImGui.Separator()
    ImGui.Text("Pulse Jet")

    _, minPulseImpulse = ImGui.SliderFloat("Min Impulse", minPulseImpulse, 1.0, 60.0)
    _, maxPulseImpulse = ImGui.SliderFloat("Max Impulse", maxPulseImpulse, 1.0, 80.0)
    _, boostDrag = ImGui.SliderFloat("Boost Drag", boostDrag, 0.5, 10.0)
    _, pulseRedirect = ImGui.SliderFloat("Redirect (keep old vel)", pulseRedirect, 0.0, 1.0)
    _, minPulseCooldown = ImGui.SliderFloat("Min Cooldown", minPulseCooldown, 0.0, 5.0)
    _, maxPulseCooldown = ImGui.SliderFloat("Max Cooldown", maxPulseCooldown, 0.0, 8.0)
    _, chargeSensMultiplier = ImGui.SliderFloat("Charge Sensitivity", chargeSensMultiplier, 0.1, 1.0)
    _, pulseChargeFov = ImGui.SliderFloat("Charge FOV Offset", pulseChargeFov, -20.0, 20.0)
    _, maxFovKick = ImGui.SliderFloat("Max FOV Kick", maxFovKick, 0.0, 30.0)
    _, fovKickDecay = ImGui.SliderFloat("FOV Kick Decay", fovKickDecay, 1.0, 20.0)

    local chargePercent = pulseChargeTime / maxPulseChargeTime

    ImGui.Text("Charging: " .. tostring(pulseCharging))
    ImGui.Text("Charge: " .. string.format("%.0f%%", chargePercent * 100.0))
    ImGui.Text("Boost Speed: " .. string.format("%.2f", GLM.V3Length(boostVelocity)))
    ImGui.Text("Cooldown: " .. string.format("%.2f / %.2f s", currentPulseCooldown, pulseCooldownTotal))
    ImGui.Text("Current Move Speed: " .. moveSpeed)

    tracy.ZoneEnd()
end