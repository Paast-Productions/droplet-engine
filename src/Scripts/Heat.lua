
local hazardComp = nil
local heatDamage = nil
local damageNormal = 20.0
local damageResistant = 10.0

local particleSystem = nil
local isActive = false
local activeTime = 5.0
local inactiveTime = 15.0
local timer = 0

local audioEmitter = nil
local isPlayingSound = false

local heatOverlay = nil
local timeSinceTrigger = math.huge
local overlayGracePeriod = 0.1

function OnStart()
    hazardComp = self:GetHazardComponent()
    heatDamage = hazardComp:GetDamage()
    newDamage = heatDamage

    isActive = true
    isPlayingSound = false
    timer = math.random() * inactiveTime

    audioEmitter = self:GetAudioEmitter("Thermal-vents")
    if audioEmitter == nil then  
        print("Unable to find emitter")
        return
    end
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

    if isActive then
        if isPlayingSound == false then
            audioEmitter:Play()
            isPlayingSound = true;
        end
    elseif !isActive then
        if isPlayingSound == true then
            audioEmitter:Stop()
            isPlayingSound = false;
        end
    end

    if heatOverlay ~= nil then
        timeSinceTrigger = timeSinceTrigger + dt
        if timeSinceTrigger >= overlayGracePeriod then
            heatOverlay:SetActiveSelf(false)
            heatOverlay = nil
        end
    end
end

function OnTrigger(dt, node)
    if node == nil or node:GetPlayerComponent() == nil then
        return
    end

    for _, child in ipairs(node:GetChildren()) do
        if child:GetName() == "HeatOverlay" then
            heatOverlay = child
            heatOverlay:SetActiveSelf(true)
            timeSinceTrigger = 0
        end
    end

    if not isActive then
        return
    end

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