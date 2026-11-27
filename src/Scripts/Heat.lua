
local json = require("../Include/json")

-- Heat configuration
local outsideTemp = 150

local damageScaling = 5.0

-- Temperature configuration
local temperatureChangeRate = 2.0
local temperatureScaling = 0.05

-- Particle system
local particleSystem = nil
local isActive = false

-- Timing configuration
local useRandomTiming = true

local activeTime = 5.0
local inactiveTime = 15.0

local activeTimeMin = 4.0
local activeTimeMax = 7.0

local inactiveTimeMin = 12.0
local inactiveTimeMax = 20.0

local timer = 0.0

-- Audio
local audioEmitter = nil
local isPlayingSound = false

-- Heat overlay
local heatOverlay = nil
local timeSinceTrigger = math.huge
local overlayGracePeriod = 0.1

-- Temperature configuration
local temperatureChangeRate = 2.0
local temperatureScaling = 0.05

local function RandomRange(min, max)
    return min + math.random() * (max - min)
end

local function GetActiveDuration()
    if useRandomTiming then
        return RandomRange(activeTimeMin, activeTimeMax)
    end

    return activeTime
end

local function GetInactiveDuration()
    if useRandomTiming then
        return RandomRange(inactiveTimeMin, inactiveTimeMax)
    end

    return inactiveTime
end

function OnStart()
    isActive = false
    timer = GetInactiveDuration()

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

    if timer <= 0.0 then
        isActive = not isActive

        if isActive then
            timer = GetActiveDuration()
        else
            timer = GetInactiveDuration()
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
    if node == nil or not isActive then
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
        local damagePerSecond = 1.0 +
            damageScaling * heatSeverity * heatSeverity

        health:TakeDamage(damagePerSecond * dt)
    end
end

function RenderUI()
    ImGui.Text("Heat Hazard Active: " .. tostring(isActive))
    ImGui.Text("Timer: " .. tostring(timer))

    
    ImGui.Separator()
    ImGui.Text("Heat Settings")

    changed, outsideTemp = ImGui.DragFloat(
        "Outside Temperature",
        outsideTemp,
        1.0
    )

    changed, damageScaling = ImGui.DragFloat(
        "Damage Scaling",
        damageScaling,
        0.1,
        0.0
    )

    ImGui.Separator()
    ImGui.Text("Temperature Settings")

    changed, temperatureChangeRate = ImGui.DragFloat(
        "Temperature Change Rate",
        temperatureChangeRate,
        0.1,
        0.0
    )

    changed, temperatureScaling = ImGui.DragFloat(
        "Temperature Scaling",
        temperatureScaling,
        0.01,
        0.0
    )

    ImGui.Separator()
    ImGui.Text("Timing Settings")

    changed, useRandomTiming = ImGui.Checkbox(
        "Use Random Timing",
        useRandomTiming
    )

    if useRandomTiming then
        ImGui.Text("Active Duration")

        changed, activeTimeMin = ImGui.DragFloat(
            "Active Minimum",
            activeTimeMin,
            0.1,
            0.1
        )

        changed, activeTimeMax = ImGui.DragFloat(
            "Active Maximum",
            activeTimeMax,
            0.1,
            0.1
        )

        ImGui.Text("Inactive Duration")

        changed, inactiveTimeMin = ImGui.DragFloat(
            "Inactive Minimum",
            inactiveTimeMin,
            0.1,
            0.1
        )

        changed, inactiveTimeMax = ImGui.DragFloat(
            "Inactive Maximum",
            inactiveTimeMax,
            0.1,
            0.1
        )

        -- Ensure minimum values do not exceed maximum values.
        activeTimeMax = math.max(activeTimeMax, activeTimeMin)
        inactiveTimeMax = math.max(inactiveTimeMax, inactiveTimeMin)
    else
        changed, activeTime = ImGui.DragFloat(
            "Active Duration",
            activeTime,
            0.1,
            0.1
        )

        changed, inactiveTime = ImGui.DragFloat(
            "Inactive Duration",
            inactiveTime,
            0.1,
            0.1
        )
    end

    ImGui.Separator()
    ImGui.Text("Visual Settings")

    changed, overlayGracePeriod = ImGui.DragFloat(
        "Overlay Duration",
        overlayGracePeriod,
        0.01,
        0.0
    )


end

function Serialize()
    local data = {
        -- Heat configuration
        outsideTemp = outsideTemp,
        damageScaling = damageScaling,

        -- Temperature configuration
        temperatureChangeRate = temperatureChangeRate,
        temperatureScaling = temperatureScaling,

        -- Timing
        useRandomTiming = useRandomTiming,
        activeTime = activeTime,
        inactiveTime = inactiveTime,
        activeTimeMin = activeTimeMin,
        activeTimeMax = activeTimeMax,
        inactiveTimeMin = inactiveTimeMin,
        inactiveTimeMax = inactiveTimeMax,

        -- Visual
        overlayGracePeriod = overlayGracePeriod
    }

    return json.encode(data)
end

function Deserialize(data)
    if data == nil or data == "" then
        return
    end

    local decoded = json.decode(data)

    if decoded == nil then
        return
    end

    -- Timing
    if decoded.useRandomTiming ~= nil then
        useRandomTiming = decoded.useRandomTiming
    end

    if decoded.activeTime ~= nil then
        activeTime = decoded.activeTime
    end

    if decoded.inactiveTime ~= nil then
        inactiveTime = decoded.inactiveTime
    end

    if decoded.activeTimeMin ~= nil then
        activeTimeMin = decoded.activeTimeMin
    end

    if decoded.activeTimeMax ~= nil then
        activeTimeMax = decoded.activeTimeMax
    end

    if decoded.inactiveTimeMin ~= nil then
        inactiveTimeMin = decoded.inactiveTimeMin
    end

    if decoded.inactiveTimeMax ~= nil then
        inactiveTimeMax = decoded.inactiveTimeMax
    end

    -- Visual
    if decoded.overlayGracePeriod ~= nil then
        overlayGracePeriod = decoded.overlayGracePeriod
    end

    if decoded.outsideTemp ~= nil then
    outsideTemp = decoded.outsideTemp
    end

    if decoded.damageScaling ~= nil then
        damageScaling = decoded.damageScaling
    end

    if decoded.temperatureChangeRate ~= nil then
        temperatureChangeRate = decoded.temperatureChangeRate
    end

    if decoded.temperatureScaling ~= nil then
        temperatureScaling = decoded.temperatureScaling
    end
end