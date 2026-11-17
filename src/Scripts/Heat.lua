local MeshToggleSystem = require("MeshToggleSystem")

local hazardComp = nil
local heatDamage = nil
local newDamage = nil

local heatOverlay = nil

function OnStart()
    hazardComp = self:GetHazardComponent()
    
    heatDamage = hazardComp:GetDamage()
    newDamage = heatDamage
end

function OnTrigger(dt, node)
    tracy.ZoneBeginN("HeatHazard OnTrigger")
    
    local health = node:GetHealthComponent()
    local player = node:GetPlayerComponent()
    local pb = nil
    if player ~= nil then
        pb = player:GetPlayerBehaviour()
    end
    
    if hazardComp ~= nil and pb ~= nil and health ~= nil then
        pb:IncreaseHeatLevel(1)
		
		if (heatOverlay == nil) then
			heatOverlay = MeshToggleSystem.GetByName("HeatOverlay")
		end
		
		MeshToggleSystem.SetActive(heatOverlay, true)

        if pb:GetHeatLevel() > pb:GetHeatResistance() then
            health:TakeDamage(heatDamage * dt)

            if health:GetHealth() <= 0 then
            end
        end
    end

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