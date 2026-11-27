local MovementModifiers = require("MovementModifiers")
local freezeConfig = require("Freeze")
local json = require("../Include/json")

local pb = nil

-- Recovery configuration
local baseThawRate = 30.0
local temperatureThawScaling = 0.1

-- Temperature recovery configuration
local defaultOutsideTemp = 5.0
local outsideTempRecoveryRate = 50.0
local droneTempRecoveryRate = 10.0

-- Ambient temperature fluctuation
local ambientTempVariation = 3.0
local ambientChangeIntervalMin = 5.0
local ambientChangeIntervalMax = 12.0

local ambientTempTarget = defaultOutsideTemp
local ambientChangeTimer = 0.0

local outsideTempChanged = false;

local function RandomRange(min, max)
    return min + math.random() * (max - min)
end

function OnStart()
    local player = self:GetPlayerComponent()

    if player ~= nil then
        pb = player:GetPlayerBehaviour()

        if pb ~= nil then
            ambientTempTarget = defaultOutsideTemp
            ambientChangeTimer = RandomRange(
                ambientChangeIntervalMin,
                ambientChangeIntervalMax
            )

            pb:SetOutsideTemp(defaultOutsideTemp)
            pb:SetDroneTemp(defaultOutsideTemp)
        end
    end
end

function OnUpdate(dt)
    if pb == nil then
        return
    end

    local timeSinceFreeze = pb:GetFreezeTimer()

    -- Wait until the recovery delay has expired.
    if timeSinceFreeze >= 0 then
        timeSinceFreeze = timeSinceFreeze - dt
        pb:SetFreezeTimer(timeSinceFreeze)
        return
    end

    -- Update ambient temperature target periodically.
    ambientChangeTimer = ambientChangeTimer - dt

    if ambientChangeTimer <= 0.0 then
        ambientTempTarget = defaultOutsideTemp + RandomRange(
            -ambientTempVariation,
            ambientTempVariation
        )

        ambientChangeTimer = RandomRange(
            ambientChangeIntervalMin,
            ambientChangeIntervalMax
        )
    end

    -- Recover outside temperature towards the current ambient target.
    local outsideTemp = pb:GetOutsideTemp()

    local outsideDifference = ambientTempTarget - outsideTemp

    local outsideChange = math.min(
        math.abs(outsideDifference),
        outsideTempRecoveryRate * dt
    )

    if outsideDifference < 0.0 then
        outsideTemp = outsideTemp - outsideChange
    else
        outsideTemp = outsideTemp + outsideChange
    end
    pb:SetOutsideTemp(outsideTemp)
    outsideTempChanged = false

    -- Recover drone temperature towards outside temperature.
    local droneTemp = pb:GetDroneTemp()
    local droneDifference = outsideTemp - droneTemp

    local droneChange = math.min(
        math.abs(droneDifference),
        droneTempRecoveryRate * dt
    )

    if droneDifference < 0.0 then
        droneTemp = droneTemp - droneChange
    else
        droneTemp = droneTemp + droneChange
    end

    pb:SetDroneTemp(droneTemp)

    -- Recover freeze level based on drone temperature.
    local currentFreeze = pb:GetFreezeLevel()
    local freezeResistance = pb:GetFreezeResistance()

    if currentFreeze > 0 then
        local freezeThreshold = -freezeResistance

        local temperatureDifference = math.max(
            0.0,
            droneTemp - freezeThreshold
        )

        -- Increase thaw rate as the drone becomes warmer.
        local currentThawRate = baseThawRate +
            temperatureDifference * temperatureThawScaling

        local decrease = math.min(
            currentFreeze,
            currentThawRate * dt
        )

        pb:IncreaseFreezeLevel(-decrease)
    end

    -- Update movement modifiers based on current freeze level.
    currentFreeze = pb:GetFreezeLevel()

    local freezePercent = math.min(
        1.0,
        currentFreeze / freezeConfig.maxFreeze
    )

    local speedMultiplier = 1.0 -
        freezePercent * (1.0 - freezeConfig.minSpeedMultiplier)

    local handlingMultiplier = 1.0 -
        freezePercent * 0.75

    MovementModifiers.SetSlow(speedMultiplier)
    MovementModifiers.SetHandling(handlingMultiplier)
end

function RenderUI()
    tracy.ZoneBeginN("FreezeController RenderUI")

    ImGui.Spacing()
    ImGui.Text("Freeze Recovery Settings")
    ImGui.Separator()

    local changed = false

    changed, baseThawRate = ImGui.DragFloat(
        "Base Thaw Rate",
        baseThawRate,
        0.1,
        0.0
    )

    changed, temperatureThawScaling = ImGui.DragFloat(
        "Temperature Thaw Scaling",
        temperatureThawScaling,
        0.01,
        0.0
    )

    ImGui.Separator()
    ImGui.Text("Temperature Recovery Settings")

    changed, defaultOutsideTemp = ImGui.DragFloat(
        "Default Outside Temperature",
        defaultOutsideTemp,
        0.1
    )

    changed, outsideTempRecoveryRate = ImGui.DragFloat(
        "Outside Temperature Recovery Rate",
        outsideTempRecoveryRate,
        0.1,
        0.0
    )

    changed, droneTempRecoveryRate = ImGui.DragFloat(
        "Drone Temperature Recovery Rate",
        droneTempRecoveryRate,
        0.1,
        0.0
    )

    ImGui.Separator()
    ImGui.Text("Ambient Temperature Variation")

    changed, ambientTempVariation = ImGui.DragFloat(
        "Temperature Variation (+/- C)",
        ambientTempVariation,
        0.1,
        0.0
    )

    changed, ambientChangeIntervalMin = ImGui.DragFloat(
        "Change Interval Minimum",
        ambientChangeIntervalMin,
        0.1,
        0.1
    )

    changed, ambientChangeIntervalMax = ImGui.DragFloat(
        "Change Interval Maximum",
        ambientChangeIntervalMax,
        0.1,
        0.1
    )

    ambientChangeIntervalMax = math.max(
        ambientChangeIntervalMax,
        ambientChangeIntervalMin
    )

    ImGui.Text("Ambient Target: " ..
        string.format("%.1f C", ambientTempTarget))

    ImGui.Text("Next Change In: " ..
        string.format("%.1f s", ambientChangeTimer))

    ImGui.Separator()

    if pb ~= nil then
        local outsideTemp = pb:GetOutsideTemp()
        local droneTemp = pb:GetDroneTemp()
        local freezeResistance = pb:GetFreezeResistance()
        local freezeLevel = pb:GetFreezeLevel()

        local freezePercent = math.min(
            100.0,
            (freezeLevel / freezeConfig.maxFreeze) * 100.0
        )

        ImGui.Text("Freeze Statistics")
        ImGui.Separator()

        ImGui.Text("Outside Temperature: " ..
            string.format("%.1f C", outsideTemp))

        ImGui.Text("Drone Temperature: " ..
            string.format("%.1f C", droneTemp))

        ImGui.Text("Freeze Resistance: " ..
            string.format("%.1f", freezeResistance))

        ImGui.Separator()

        ImGui.Text("Freeze Level: " ..
            string.format("%.1f / %.1f", freezeLevel, freezeConfig.maxFreeze))

        ImGui.Text("Freeze Progress: " ..
            string.format("%.1f%%", freezePercent))
    else
        ImGui.Text("PlayerBehaviour unavailable")
    end

    tracy.ZoneEnd()
end

function Serialize()
    local data = {
        -- Freeze recovery
        baseThawRate = baseThawRate,
        temperatureThawScaling = temperatureThawScaling,

        -- Temperature recovery
        defaultOutsideTemp = defaultOutsideTemp,
        outsideTempRecoveryRate = outsideTempRecoveryRate,
        droneTempRecoveryRate = droneTempRecoveryRate,
        -- Ambient temperature variation
        ambientTempVariation = ambientTempVariation,
        ambientChangeIntervalMin = ambientChangeIntervalMin,
        ambientChangeIntervalMax = ambientChangeIntervalMax
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

    -- Freeze recovery
    if decoded.baseThawRate ~= nil then
        baseThawRate = decoded.baseThawRate
    end

    if decoded.temperatureThawScaling ~= nil then
        temperatureThawScaling = decoded.temperatureThawScaling
    end

    -- Temperature recovery
    if decoded.defaultOutsideTemp ~= nil then
        defaultOutsideTemp = decoded.defaultOutsideTemp
    end

    if decoded.outsideTempRecoveryRate ~= nil then
        outsideTempRecoveryRate = decoded.outsideTempRecoveryRate
    end

    if decoded.droneTempRecoveryRate ~= nil then
        droneTempRecoveryRate = decoded.droneTempRecoveryRate
    end

    if decoded.ambientTempVariation ~= nil then
    ambientTempVariation = decoded.ambientTempVariation
    end

    if decoded.ambientChangeIntervalMin ~= nil then
        ambientChangeIntervalMin = decoded.ambientChangeIntervalMin
    end

    if decoded.ambientChangeIntervalMax ~= nil then
        ambientChangeIntervalMax = decoded.ambientChangeIntervalMax
    end
end