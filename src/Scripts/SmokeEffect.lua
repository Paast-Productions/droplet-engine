local pTransforms = nil
local pStates = nil

-- Particle Settings
local gravity = { x = 0, y = 0.4, z = 0 }
local initialVelocityRangeHorizontal = { min = -0.5, max = 0.5 }
local initialVelocityRangeVertical = { min = 0.2, max = 0.8 }
local initialSizeRange = { min = 0.025, max = 0.1 }
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
function ParticleState:new()
	local t = setmetatable({}, { __index = ParticleState })

	t.time = 0
	t.position = { x = 0, y = 0, z = 0 }

	t.lifetime = Lerp(lifetimeRange.min, lifetimeRange.max, math.random())
	t.initialSize = Lerp(initialSizeRange.min, initialSizeRange.max, math.random())
	t.velocity = {
		x = Lerp(initialVelocityRangeHorizontal.min, initialVelocityRangeHorizontal.max, math.random()),
		y = Lerp(initialVelocityRangeVertical.min, initialVelocityRangeVertical.max, math.random()),
		z = Lerp(initialVelocityRangeHorizontal.min, initialVelocityRangeHorizontal.max, math.random())
	}

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
	t:SetPosition(Vec3.new(s.position.x, s.position.y, s.position.z))
	t:SetScale(Vec3.new(size, size, size))

	return (s.time > s.lifetime)
end

function GenSmokeParticles()
    pTransforms = {}
    pStates = {}
    local smokeParticleNodes = self:GetChildren()
    
    -- Loop through children and fetch their transforms to smokeParticles
    for index, value in ipairs(smokeParticleNodes) do
        pTransforms[index] = value:GetTransform()
        pStates[index] = ParticleState:new()
    end
end


function OnStart()
    GenSmokeParticles()

	if pStates == nil then
		return
	end
end

function OnUpdate(dt)
    if pTransforms == nil then
        GenSmokeParticles()

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
			pStates[i] = ParticleState:new()
		end
    end
end

function RenderUI()
	return
end
