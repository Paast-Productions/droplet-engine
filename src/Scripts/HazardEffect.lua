local json = require("../Include/json")

local pTransforms = nil
local pStates = nil

-- Particle Settings
local space = TransformSpace.Local
local gravity = { x = 0, y = 0.4, z = 0 }
local spawnBox = { min = { x = -0.5, y = -0.5, z = -0.5 }, max = { x = 0.5, y = 0.5, z = 0.5 } }
local initialVelocityRange = { min = { x = -0.5, y = -0.5, z = -0.5 }, max = { x = 0.5, y = 0.5, z = 0.5 } }
local sizeRange = { min = 0.025, max = 0.1 }
local lifetimeRange = { min = 1.0, max = 3.0 }

function Lerp(a, b, t)
	return a + (b - a) * t
end

function LerpVec3(a, b, t)
	return {
		x = Lerp(a.x, b.x, t),
		y = Lerp(a.y, b.y, t),
		z = Lerp(a.z, b.z, t)
	}
end

function RandVec3(min, max)
	return {
		x = Lerp(min.x, max.x, math.random()),
		y = Lerp(min.y, max.y, math.random()),
		z = Lerp(min.z, max.z, math.random())
	}
end

local ParticleState = {}
function ParticleState:new(trans)
	local t = setmetatable({}, { __index = ParticleState })

	t.time = 0
	t.position = {
		x = Lerp(spawnBox.min.x, spawnBox.max.x, math.random()),
		y = Lerp(spawnBox.min.y, spawnBox.max.y, math.random()),
		z = Lerp(spawnBox.min.z, spawnBox.max.z, math.random())
	}

	t.lifetime = Lerp(lifetimeRange.min, lifetimeRange.max, math.random())
	t.initialSize = Lerp(sizeRange.min, sizeRange.max, math.random())
	t.velocity = {
		x = Lerp(initialVelocityRange.min.x, initialVelocityRange.max.x, math.random()),
		y = Lerp(initialVelocityRange.min.y, initialVelocityRange.max.y, math.random()),
		z = Lerp(initialVelocityRange.min.z, initialVelocityRange.max.z, math.random())
	}

	trans:SetPosition(Vec3.new(t.position.x, t.position.y, t.position.z), TransformSpace.Local)
	t.position = trans:GetPosition(space)

	local rotation = Vec3.new(math.random() * math.pi * 2, math.random() * math.pi * 2, math.random() * math.pi * 2)
	trans:SetEuler(rotation)

    return t
end


function UpdateParticle(dt, t, s)
	s.time = s.time + dt

	-- Update particle position based on velocity
	s.position.x = s.position.x + s.velocity.x * dt
	s.position.y = s.position.y + s.velocity.y * dt
	s.position.z = s.position.z + s.velocity.z * dt

	-- Update particle velocity based on gravity
	s.velocity.x = s.velocity.x + gravity.x * dt
	s.velocity.y = s.velocity.y + gravity.y * dt
	s.velocity.z = s.velocity.z + gravity.z * dt

	-- Lerp size towards zero over lifetime
	local size = Lerp(s.initialSize, 0, s.time / s.lifetime)
	size = math.sqrt(size)

	-- Apply the calculated size to the particle's transform
	t:SetPosition(Vec3.new(s.position.x, s.position.y, s.position.z), space)
	t:SetScale(Vec3.new(size, size, size))

	return (s.time > s.lifetime)
end

function GenParticles()
    pTransforms = {}
    pStates = {}
    local particleNodes = self:GetChildren()
    
    -- Loop through children and fetch their transforms to particles
    for index, value in ipairs(particleNodes) do
        pTransforms[index] = value:GetTransform()
        pStates[index] = ParticleState:new(pTransforms[index])
    end
end


function OnStart()
    GenParticles()

	if pStates == nil then
		return
	end
end

function OnUpdate(dt)
    if pTransforms == nil then
        GenParticles()

        if pTransforms == nil then
            return  
        end
    end

	if pStates == nil then
		return
	end
    
    for i, t in ipairs(pTransforms) do
        local dead = UpdateParticle(dt, t, pStates[i])

		if dead then
			pStates[i] = ParticleState:new(t)
		end
    end
end

function RenderUI()
	local worldSpace = (space == TransformSpace.World)
	_, worldSpace = ImGui.Checkbox("World Space", worldSpace)
	space = worldSpace and TransformSpace.World or TransformSpace.Local	

	local vec3 = Vec3.new(gravity.x, gravity.y, gravity.z)
    _, vec3 = ImGui.DragFloat3("Gravity", vec3, 0.01)
	gravity.x = vec3.x
	gravity.y = vec3.y
	gravity.z = vec3.z

	vec3.x = spawnBox.min.x
	vec3.y = spawnBox.min.y
	vec3.z = spawnBox.min.z
    _, vec3 = ImGui.DragFloat3("Spawn Box Min", vec3, 0.01)
	spawnBox.min.x = vec3.x
	spawnBox.min.y = vec3.y
	spawnBox.min.z = vec3.z

	vec3.x = spawnBox.max.x
	vec3.y = spawnBox.max.y
	vec3.z = spawnBox.max.z
    _, vec3 = ImGui.DragFloat3("Spawn Box Max", vec3, 0.01)
	spawnBox.max.x = vec3.x
	spawnBox.max.y = vec3.y
	spawnBox.max.z = vec3.z

	vec3.x = initialVelocityRange.min.x
	vec3.y = initialVelocityRange.min.y
	vec3.z = initialVelocityRange.min.z
    _, vec3 = ImGui.DragFloat3("Initial Velocity Min", vec3, 0.01)
	initialVelocityRange.min.x = vec3.x
	initialVelocityRange.min.y = vec3.y
	initialVelocityRange.min.z = vec3.z

	vec3.x = initialVelocityRange.max.x
	vec3.y = initialVelocityRange.max.y
	vec3.z = initialVelocityRange.max.z
    _, vec3 = ImGui.DragFloat3("Initial Velocity Max", vec3, 0.01)
	initialVelocityRange.max.x = vec3.x
	initialVelocityRange.max.y = vec3.y
	initialVelocityRange.max.z = vec3.z

	local vec2 = Vec2.new(sizeRange.min, sizeRange.max)
    _, vec2 = ImGui.DragFloat2("Size Range", vec2, 0.01)
	sizeRange.min = vec2.x
	sizeRange.max = vec2.y

	vec2.x = lifetimeRange.min
	vec2.y = lifetimeRange.max
    _, vec2 = ImGui.DragFloat2("Lifetime Range", vec2, 0.01)
	lifetimeRange.min = vec2.x
	lifetimeRange.max = vec2.y

	-- Draw spawn box
	local trans = self:GetTransform()
	local pos = trans:GetPosition(TransformSpace.World)
	local rot = trans:GetRotation(TransformSpace.World)

	local center = Vec3.new(
		(spawnBox.min.x + spawnBox.max.x) * 0.5, 
		(spawnBox.min.y + spawnBox.max.y) * 0.5, 
		(spawnBox.min.z + spawnBox.max.z) * 0.5
	)

	center = GLM.V3Rotate(center, rot)
	center.x = center.x + pos.x
	center.y = center.y + pos.y
	center.z = center.z + pos.z

	local extents = Vec3.new(
		(spawnBox.max.x - spawnBox.min.x) * 0.5, 
		(spawnBox.max.y - spawnBox.min.y) * 0.5, 
		(spawnBox.max.z - spawnBox.min.z) * 0.5
	)

	local obb = OBB.new(center, extents, rot)
	DebugDraw.DrawOBB(obb, Vec4.new(0, 1, 0, 1), false, true)
end

function Serialize()
    local data = {
		space = space,
        grav = gravity,
        spawnBox = spawnBox,
        velRange = initialVelocityRange,
        sizeRange = sizeRange,
        lifetimeRange = lifetimeRange
    }
    return json.encode(data)
end

function Deserialize(data)
    local decoded = json.decode(data)
    if decoded then
        if decoded.space then
            space = decoded.space
        end
        if decoded.grav then
            gravity = decoded.grav
        end
        if decoded.spawnBox then
            spawnBox = decoded.spawnBox
        end
        if decoded.velRange then
            initialVelocityRange = decoded.velRange
        end
        if decoded.sizeRange then
            sizeRange = decoded.sizeRange
        end
        if decoded.lifetimeRange then
            lifetimeRange = decoded.lifetimeRange
        end
    end
end
