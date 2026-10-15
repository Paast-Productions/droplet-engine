
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

local moveSpeed = 0.0
local turnSpeed = 0.0


--
-- Member Functions
--

function NewTargetPoint()
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
end

function IsWithinTarget()
	local pos = trans:GetPosition(TransformSpace.World)
	local dist = GLM.V3Distance(pos, targetPoint)
	return dist <= targetAcceptanceRadius
end

function UpdateVelocity(dt)
	local pos = trans:GetPosition(TransformSpace.World)
	
	local dirToTarget = GLM.V3Sub(targetPoint, pos)
	local distToTarget = GLM.V3Length(dirToTarget)

	if distToTarget > 0 then
		dirToTarget = GLM.V3MulF(dirToTarget, 1 / distToTarget)
	end

	moveSpeed = GLM.V3Length(dirToTarget) * maxMoveSpeed
	turnSpeed = GLM.V3Length(GLM.V3Cross(trans:GetForward(), dirToTarget)) * maxTurnSpeed
end

function ApplyVelocity(dt)
	local toMove = GLM.V3MulF(GLM.V3MulF(trans:GetForward(), moveSpeed), dt)
	trans:Move(toMove, TransformSpace.World)

	local toRotate = GLM.V3MulF(GLM.V3MulF(trans:GetUp(), turnSpeed), dt)
	trans:RotateEuler(toRotate, TransformSpace.World)
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

	UpdateVelocity(dt)
	ApplyVelocity(dt)

end

function RenderUI()
	_, orbitPoint = ImGui.DragFloat3("Orbit Point", orbitPoint, 0.2)
	_, targetRadius = ImGui.DragFloat("Target Radius", targetRadius, 0.1)
	_, targetAcceptanceRadius = ImGui.DragFloat("Target Acceptance Radius", targetAcceptanceRadius, 0.1)

	_, maxMoveSpeed = ImGui.DragFloat("Move Speed", maxMoveSpeed, 0.1)
	_, moveSpeedRateOfChange = ImGui.DragFloat("Move Rate of Change", moveSpeedRateOfChange, 0.01)

	_, maxTurnSpeed = ImGui.DragFloat("Turn Speed", maxTurnSpeed, 0.01)
	_, turnSpeedRateOfChange = ImGui.DragFloat("Turn Rate of Change", turnSpeedRateOfChange, 0.01)
end
