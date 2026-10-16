-- Member Variables

local trans = nil
local center = Vec3.new(0.0, 0.0, 0.0)

local radius = 3.0
local angularSpeed = 1.0
local angle = 0.0
local height = 0.0

-- Auto Functions

function OnStart()
-- 'trans' should be injected into this instance's environment by C++.
if trans == nil then
print("circleMovement: missing transform")
return
end

local position = trans:GetPosition(TransformSpace.World)

center = Vec3.new(position.x, position.y, position.z)
height = position.y

-- Start at the node's current angle around the center.
local offsetX = position.x - center.x
local offsetZ = position.z - center.z

angle = math.atan(offsetX, offsetZ)

end

function OnUpdate(dt)
if trans == nil then
return
end

angle = angle + angularSpeed * dt

local x = center.x + math.sin(angle) * radius
local z = center.z + math.cos(angle) * radius

local currentPosition = trans:GetPosition(TransformSpace.World)

local targetPosition = Vec3.new(x, height, z)
local movement = GLM.V3Sub(targetPosition, currentPosition)

trans:Move(movement, TransformSpace.World)

end