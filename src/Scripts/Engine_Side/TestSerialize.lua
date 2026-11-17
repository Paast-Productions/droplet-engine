local json = require("../Include/json")

-- Particle Settings
local gravity = { x = 0, y = 0.4, z = 0 }
local initialVelocityRangeHorizontal = { min = -0.5, max = 0.5 }
local initialVelocityRangeVertical = { min = 0.2, max = 0.8 }
local initialSizeRange = { min = 0.025, max = 0.1 }
local lifetimeRange = { min = 1.0, max = 3.0 }

function OnStart()
	return
end

function OnUpdate(dt)
	return
end

function RenderUI()
	local vec3 = Vec3.new(gravity.x, gravity.y, gravity.z)
    _, vec3 = ImGui.DragFloat3("Gravity", vec3, 0.01)
	gravity.x = vec3.x
	gravity.y = vec3.y
	gravity.z = vec3.z

	
	local vec2 = Vec2.new(initialVelocityRangeHorizontal.min, initialVelocityRangeHorizontal.max)
    _, vec2 = ImGui.DragFloat2("Horizontal Velocity Range", vec2, 0.01)
	initialVelocityRangeHorizontal.min = vec2.x
	initialVelocityRangeHorizontal.max = vec2.y

	vec2.x = initialVelocityRangeVertical.min
	vec2.y = initialVelocityRangeVertical.max
    _, vec2 = ImGui.DragFloat2("Vertical Velocity Range", vec2, 0.01)
	initialVelocityRangeVertical.min = vec2.x
	initialVelocityRangeVertical.max = vec2.y

	vec2.x = initialSizeRange.min
	vec2.y = initialSizeRange.max
    _, vec2 = ImGui.DragFloat2("Size Range", vec2, 0.01)
	initialSizeRange.min = vec2.x
	initialSizeRange.max = vec2.y

	vec2.x = lifetimeRange.min
	vec2.y = lifetimeRange.max
    _, vec2 = ImGui.DragFloat2("Lifetime Range", vec2, 0.01)
	lifetimeRange.min = vec2.x
	lifetimeRange.max = vec2.y
end

function Serialize()
    local data = {
        grav = gravity,
        velRangeHor = initialVelocityRangeHorizontal,
        velRangeVer = initialVelocityRangeVertical,
        sizeRange = initialSizeRange,
        lifetimeRange = lifetimeRange
    }
    return json.encode(data)
end

function Deserialize(data)
    local decoded = json.decode(data)
    if decoded then
        if decoded.grav then
            gravity = decoded.grav
        end
        if decoded.velRangeHor then
            initialVelocityRangeHorizontal = decoded.velRangeHor
        end
        if decoded.velRangeVer then
            initialVelocityRangeVertical = decoded.velRangeVer
        end
        if decoded.sizeRange then
            initialSizeRange = decoded.sizeRange
        end
        if decoded.lifetimeRange then
            lifetimeRange = decoded.lifetimeRange
        end
    end
end
