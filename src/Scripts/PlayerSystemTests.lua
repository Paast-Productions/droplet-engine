
--
-- Member Variables
--

local trans = nil
local healthComp = nil
local playerComp = nil
local pb = nil

local hasHealthComponent = false
local hasPlayerComponent = false

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

local mouseSensitivity = 0.0025   -- radians per pixel
local maxPitch = math.rad(80.0)   -- pitch limit up/down
local invertY = false
local requireRightMouse = true    -- only rotate while right mouse is held

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

function WrapAngle(angle)
    while angle > math.pi do
        angle = angle - 2.0 * math.pi
    end

    while angle < -math.pi do
        angle = angle + 2.0 * math.pi
    end

    return qangle
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

    trans:Rotate(GLM.AngleAxis(trans:GetRight(), pitchDelta))
    trans:Rotate(GLM.AngleAxis(Vec3.new(0.0, 1.0, 0.0), yawDelta), TransformSpace.World)
end
--
-- Auto Functions
--

function OnStart()
    tracy.ZoneBeginN("Drone_movement OnStart")

    print("New Drone_movement")

    trans = self:GetTransform()

    -- Check for HealthComponent
    healthComp = self:GetHealthComponent()
    hasHealthComponent = healthComp ~= nil

    if not hasHealthComponent then
        print("[Drone_movement] Warning: HealthComponent not found.")
    end

    -- Check for PlayerComponent
    playerComp = self:GetPlayerComponent()
    hasPlayerComponent = playerComp ~= nil

    if not hasPlayerComponent then
        print("[Drone_movement] Warning: PlayerComponent not found.")
    else
        -- Check for PlayerBehaviour
        pb = playerComp:GetPlayerBehaviour()

        if pb == nil then
            print("[Drone_movement] Warning: PlayerBehaviour not found.")
        elseif hasHealthComponent then
            healthComp:SetMaxHealth(pb:GetMaxHealth())
        end
    end

    moveSpeed = 2.5
    mouseSmoothTime = 0.050

    tracy.ZoneEnd()
end

function OnUpdate(dt)
    tracy.ZoneBeginN("Drone_movement OnUpdate")

    ApplyMouseLook(dt)
    if Input:KeyPressed(Key.Q) then
        if (mouseLock == false) then
            mouseLock = true
        else
            mouseLock = false
        end
    end
	if Input:KeyHeld(Key.W) then
        print("W pressed")
        local toMove = GLM.V3MulF(GLM.V3MulF(trans:GetForward(), maxMoveSpeed), dt)
        trans:Move(toMove, TransformSpace.World)
    end

    if Input:KeyHeld(Key.A) then
        print("A pressed")
        local toMove = GLM.V3MulF(GLM.V3MulF(trans:GetForward(), 1.0), dt)
        -- trans:Move(toMove, TransformSpace.World)
    end

    if Input:KeyHeld(Key.S) then
        print("S pressed")
        local toMove = GLM.V3MulF(GLM.V3MulF(trans:GetForward(), -maxMoveSpeed), dt)
        trans:Move(toMove, TransformSpace.World)
    end

    if Input:KeyHeld(Key.D) then
        print("D pressed")
        local toMove = GLM.V3MulF(GLM.V3MulF(trans:GetForward(), 1.0), dt)
        -- trans:Move(toMove, TransformSpace.World)
    end

    if Input:KeyHeld(Key.K) then
    if pb ~= nil and healthComp ~= nil then
        print("heat increased :()")

        pb:IncreaseHeatLevel(1)

        if pb:GetHeatLevel() > pb:GetHeatResistance() then
            print("taking damage:==)")

            healthComp:TakeDamage(1)

            if healthComp:GetHealth() <= 0 then
                print("DEAD :P")
            end
        end
    end
end

    tracy.ZoneEnd()
end

function RenderUI()
    tracy.ZoneBeginN("Drone_movement RenderUI")

	ImGui.Spacing()
	_, maxMoveSpeed = ImGui.DragFloat("Move Speed", maxMoveSpeed, 0.1)
    _, mouseSmoothTime = ImGui.DragFloat("Mouse Smoothing (s)", mouseSmoothTime, 0.005, 0.0, 0.3)
	ImGui.Text("Current Move Speed: " .. moveSpeed)
	ImGui.Text("Current Turn Speed: (" .. turnSpeedX .. ", " .. turnSpeedY .. ")")
	ImGui.Text("Current Target Point: (" .. targetPoint.x .. ", " .. targetPoint.y .. ", " .. targetPoint.z .. ")")
	ImGui.Text("Distance to Target: " .. GLM.V3Length(GLM.V3Sub(trans:GetPosition(TransformSpace.World), targetPoint)))

    tracy.ZoneEnd()
end
