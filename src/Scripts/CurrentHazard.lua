local hazardComp = nil
local direction = nil
local currentStrength = nil
local velocity = Vec3.new(0.0, 0.0, 0.0)
local newStrength = nil
local newDirection = Vec3.new(0.0, 0.0, 0.0)

function OnStart()
    hazardComp = self:GetHazardComponent()
    if hazardComp ~= nil then
		direction = hazardComp:GetDirection()
		newDirection = direction
		currentStrength = hazardComp:GetDamage()
		newStrength = currentStrength
	else
		print(self:GetName() .. " has no HazardComponent")
	end
end

function OnUpdate(dt)
    tracy.ZoneBeginN("CurrentHazard OnUpdate")
    
    tracy.ZoneEnd()
end

function OnTrigger(dt, node)
    tracy.ZoneBeginN("CurrentHazard OnTrigger")

    local player = node:GetPlayerComponent()
    local pb = nil
    if player ~= nil then
        pb = player:GetPlayerBehaviour()
    end

    if hazardComp ~= nil and pb ~= nil then
        velocity = GLM.V3MulF(direction, currentStrength * dt)
        local t = node:GetTransform()
        t:Move(velocity)
    end

    tracy.ZoneEnd()
end

function RenderUI()
    if hazardComp ~= nil then
		local hasChanged = false
				hasChanged, newStrength = ImGui.DragFloat("Strength", newStrength, 0.1, 0.0)
		hazardComp:SetDamage(newStrength)
		if hasChanged then
			currentStrength = newStrength
		end
		
        hasChanged, newDirection = ImGui.DragFloat3("Direction", newDirection, 0.1)
        hazardComp:SetDirection(newDirection)
		if hasChanged then
			direction = newDirection
		end
    end
end