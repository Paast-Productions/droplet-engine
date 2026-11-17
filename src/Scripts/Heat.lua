
local hazardComp = nil
local heatDamage = nil
local damageNormal = 20.0
local damageResistant = 10.0

function OnStart()
    hazardComp = self:GetHazardComponent()
    heatDamage = hazardComp:GetDamage()
    newDamage = heatDamage
end

function OnTrigger(dt, node)
    tracy.ZoneBeginN("HeatHazard OnTrigger")

    if node == nil then
        tracy.ZoneEnd()
        return
    end

    local health = node:GetHealthComponent()
    local player = node:GetPlayerComponent()

    if health == nil or player == nil then
        tracy.ZoneEnd()
        return
    end

    local pb = player:GetPlayerBehaviour()

    if pb == nil then
        tracy.ZoneEnd()
        return
    end

    -- Increase heat while inside the hazard.
    pb:SetHeatLevel(100)

    -- Determine damage based on heat resistance.
    local damagePerSecond = damageNormal

    if pb:GetHeatResistance() >= 100 then
        damagePerSecond = damageResistant
    end

    -- Apply frame-rate independent damage.
    health:TakeDamage(damagePerSecond * heatDamage * dt)

    tracy.ZoneEnd()
end

function RenderUI()
	if hazardComp ~= nil then
		local hasChanged = false
		hasChanged, newDamage = ImGui.DragFloat("Damage", newDamage, 0.1, 0.0)
		hazardComp:SetDamage(newDamage)
		if hasChanged then
			heatDamage = newDamage
		end
	end
end