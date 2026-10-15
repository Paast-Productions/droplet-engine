
--
-- Member Variables
--

local trans = self:GetTransform()

local orbitPoint = Vec3.new(0.0, 0.0, 0.0)
local targetPoint = Vec3.new(0.0, 0.0, 0.0)
local targetRadius = 5.0
local targetAcceptanceRadius = 0.5

local maxMoveSpeed = 5.0
local maxTurnSpeed = 1.0
local moveSpeedRateOfChange = 0.1
local turnSpeedRateOfChange = 0.1

local linVel = Vec3.new(0.0, 0.0, 0.0)
local angVel = Vec3.new(0.0, 0.0, 0.0)


--
-- Member Functions
--

function NewTargetPoint()
	-- Random point on a sphere using spherical coordinates
	local theta = math.random() * 2 * math.pi
    local phi = math.acos(2 * math.random() - 1)

    -- Random distance, uniform by volume
    local r = targetRadius * math.random()^(1/3)

    local x = r * math.sin(phi) * math.cos(theta)
    local y = r * math.sin(phi) * math.sin(theta)
    local z = r * math.cos(phi)

    targetPoint.x = orbitPoint.x + x
    targetPoint.y = orbitPoint.y + y
    targetPoint.z = orbitPoint.z + z
end

function IsWithinTarget()
	local pos = trans:GetPosition(TransformSpace.World)
	local distSqr = (pos.x - targetPoint.x)^2 + (pos.y - targetPoint.y)^2 + (pos.z - targetPoint.z)^2
	return distSqr <= targetAcceptanceRadius^2
end

function UpdateVelocity(dt)
	local pos = trans:GetPosition(TransformSpace.World)

	-- Linear velocity
	local toTarget = Vec3.new(targetPoint.x - pos.x, targetPoint.y - pos.y, targetPoint.z - pos.z)
	local dist = math.sqrt(toTarget.x^2 + toTarget.y^2 + toTarget.z^2)

	if dist > 0 then
		toTarget.x = toTarget.x / dist
		toTarget.y = toTarget.y / dist
		toTarget.z = toTarget.z / dist
	end

	linVel.x = toTarget.x * moveSpeed
	linVel.y = toTarget.y * moveSpeed
	linVel.z = toTarget.z * moveSpeed

	-- Angular velocity (simple approach: rotate around up axis)
	local forward = trans:GetRotation():GetForward()
	local right = trans:GetRotation():GetRight()
	local up = trans:GetRotation():GetUp()

	local desiredForward = Vec3.new(toTarget.x, toTarget.y, toTarget.z)
	local dot = forward.x * desiredForward.x + forward.y * desiredForward.y + forward.z * desiredForward.z
	local angleDiff = math.acos(dot)

	if angleDiff > 0.01 then
		local crossX = forward.y * desiredForward.z - forward.z * desiredForward.y
		local crossY = forward.z * desiredForward.x - forward.x * desiredForward.z
		local crossZ = forward.x * desiredForward.y - forward.y * desiredForward.x

		angVel.x = crossX * turnSpeed
		angVel.y = crossY * turnSpeed
		angVel.z = crossZ * turnSpeed
	else
		angVel.x = 0.0
		angVel.y = 0.0
		angVel.z = 0.0
	end

end

function ApplyVelocity(dt)
	local toMove = Vec3.new(linVel.x * dt, linVel.y * dt, linVel.z * dt)
	trans:Move(toMove, TransformSpace.World)

	local toRotate = Vec3.new(angVel.x * dt, angVel.y * dt, angVel.z * dt)
	trans:Rotate(toRotate, TransformSpace.World)

end


--
-- Auto Functions
--

function OnStart()
	print("New Fih")
	trans = self:GetTransform()
	orbitPoint = trans:GetPosition(TransformSpace.World)
	NewTargetPoint()
end

function OnUpdate(dt)
	-- TODO

	if IsWithinTarget() then
		NewTargetPoint()
	end


end

function RenderUI()
	_, orbitPoint = ImGui.DragFloat3("Orbit Point", orbitPoint, 0.2)
	_, targetRadius = ImGui.DragFloat("Target Radius", targetRadius, 0.1)
	_, targetAcceptanceRadius = ImGui.DragFloat("Target Acceptance Radius", targetAcceptanceRadius, 0.1)

	_, maxMoveSpeed = ImGui.DragFloat("Move Speed", maxMoveSpeed, 0.1)
	_, maxTurnSpeed = ImGui.DragFloat("Turn Speed", maxTurnSpeed, 0.01)

	_, moveSpeedRateOfChange = ImGui.DragFloat("Move Rate of Change", moveSpeedRateOfChange, 0.01)
	_, turnSpeedRateOfChange = ImGui.DragFloat("Turn Rate of Change", turnSpeedRateOfChange, 0.01)
end
