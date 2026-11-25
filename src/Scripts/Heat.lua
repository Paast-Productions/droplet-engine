
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

local temperatureChangeRate = 2.0       
local temperatureScaling = 0.05

function OnStart()
    hazardComp = self:GetHazardComponent()

    if hazardComp == nil then
        print("ERROR: HazardComponent missing!")
        return
    end

    heatDamage = hazardComp:GetDamage()
    newDamage = heatDamage

    isActive = false
    timer = inactiveTime

    audioEmitter = self:GetAudioEmitter("Thermal-vents")

    if audioEmitter == nil then
        print("Unable to find emitter (Heat.lua)")
    end
end

function OnUpdate(dt)
    -- Find particle system.
    if particleSystem == nil then
        local parent = self:GetParent()

        if parent ~= nil then
            for _, child in ipairs(parent:GetChildren()) do
                if child:GetName() == "Heat ParticleSystem" then
                    particleSystem = child
                    break
                end
            end
        end
    end

    -- Update activation timer.
    timer = timer - dt

    if timer <= 0 then
        isActive = not isActive

        if isActive then
            timer = activeTime
        else
            timer = inactiveTime
        end
    end

    -- Synchronize particles.
    if particleSystem ~= nil then
        particleSystem:SetActiveSelf(isActive)
    end

    -- Synchronize audio.
    if audioEmitter ~= nil then
        if isActive and not isPlayingSound then
            audioEmitter:Play()
            isPlayingSound = true
        elseif not isActive and isPlayingSound then
            audioEmitter:Stop()
            isPlayingSound = false
        end
    end

    -- Handle heat overlay.
    if heatOverlay ~= nil then
        timeSinceTrigger = timeSinceTrigger + dt

        if timeSinceTrigger >= overlayGracePeriod then
            heatOverlay:SetActiveSelf(false)
            heatOverlay = nil
        end
    end
end



function OnTrigger(dt, node)
    if node == nil or hazardComp == nil or not isActive then
        return
    end

    -- Find and activate heat overlay.
    for _, child in ipairs(node:GetChildren()) do
        if child:GetName() == "HeatOverlay" then
            heatOverlay = child
            heatOverlay:SetActiveSelf(true)
            timeSinceTrigger = 0
        end
    end

    local health = node:GetHealthComponent()
    local player = node:GetPlayerComponent()

    if health == nil or player == nil then
        return
    end

    local pb = player:GetPlayerBehaviour()

    if pb == nil then
        return
    end

    local droneTemp = pb:GetDroneTemp()
    local outsideTemp = 100.0 * heatDamage

    pb:SetOutsideTemp(outsideTemp)

    local heatResistance = pb:GetHeatResistance()

    -- Calculate how far outside temperature exceeds resistance.
    local outsideHeatDifference = math.max(
        0.0,
        outsideTemp - heatResistance
    )

    -- Scale temperature increase based on resistance difference.
    local temperatureMultiplier = 1.0 +
        outsideHeatDifference * temperatureScaling

    local currentTemperatureRate =
        temperatureChangeRate * temperatureMultiplier

    -- Gradually move drone temperature towards outside temperature.
    local temperatureDifference = outsideTemp - droneTemp

    local temperatureChange = math.min(
        math.abs(temperatureDifference),
        currentTemperatureRate * dt
    )

    if temperatureDifference > 0.0 then
        droneTemp = droneTemp + temperatureChange
    else
        droneTemp = droneTemp - temperatureChange
    end

    pb:SetDroneTemp(droneTemp)

    -- Calculate how far above heat resistance the drone is.
    local heatDifference = math.max(
        0.0,
        droneTemp - heatResistance
    )

    if heatDifference > 0.0 then
        -- Normalize temperature difference.
        local heatSeverity = math.min(heatDifference / 50.0, 1.0)

        -- Apply progressively increasing damage.
        local damagePerSecond = damageNormal * heatSeverity

        health:TakeDamage(damagePerSecond * heatDamage * dt)
    end
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