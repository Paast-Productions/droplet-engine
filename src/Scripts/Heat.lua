local hazardComp = nil
local heatDamage = nil
local newDamage = nil

local particleSystem = nil
local isActive = false
local activeTime = 5.0
local inactiveTime = 15.0
local timer = 0

function OnStart()
    hazardComp = self:GetHazardComponent()
    heatDamage = hazardComp:GetDamage()
    newDamage = heatDamage

    isActive = true
    timer = math.random() * inactiveTime
end

function OnUpdate(dt)
    if particleSystem == nil then
        local parent = self:GetParent()

        if parent ~= nil then
            local children = parent:GetChildren()

            for i, child in ipairs(children) do
                if child:GetName() == "Heat ParticleSystem" then
                    particleSystem = child
                    break
                end
            end
        end
    end
    
    timer = timer - dt
    if timer <= 0 then
        if isActive then
            isActive = false
            timer = timer + inactiveTime
        else
            isActive = true
            timer = timer + activeTime
        end

        if particleSystem ~= nil then
            particleSystem:SetActiveSelf(isActive)
        end
    end
end

function OnTrigger(dt, node)
    if not isActive then
        return
    end

    tracy.ZoneBeginN("HeatHazard OnTrigger")
    
    local health = node:GetHealthComponent()
    local player = node:GetPlayerComponent()
    local pb = nil
    if player ~= nil then
        pb = player:GetPlayerBehaviour()
    end
    
    if hazardComp ~= nil and pb ~= nil and health ~= nil then

        pb:IncreaseHeatLevel(1)

        if pb:GetHeatLevel() > pb:GetHeatResistance() then
            health:TakeDamage(heatDamage * dt)

            if health:GetHealth() <= 0 then
            end
        end
    end

    tracy.ZoneEnd()
end

function RenderUI()
    ImGui.Text("Heat Hazard Active: " .. tostring(isActive))
    ImGui.Text("Timer: " .. tostring(timer))
    
	if hazardComp ~= nil then
		local hasChanged = false
		hasChanged, newDamage = ImGui.DragFloat("Damage", newDamage, 0.1, 0.0)
		hazardComp:SetDamage(newDamage)
		if hasChanged then
			heatDamage = newDamage
		end
	end
end