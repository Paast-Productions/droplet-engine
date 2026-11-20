
local MovementModifiers = require("MovementModifiers")
local freezeConfig = require("Freeze")

local pb = nil

local recoveryDelay = 3.0
local baseThawRate = 30.0
local temperatureThawScaling = 0.1

-- Temperature recovery configuration
local defaultOutsideTemp = 5.0
local outsideTempRecoveryRate = 50
local droneTempRecoveryRate = 10

function OnStart()
    local player = self:GetPlayerComponent()

    if player ~= nil then
        pb = player:GetPlayerBehaviour()
        pb:SetOutsideTemp(defaultOutsideTemp)
        pb:SetDroneTemp(defaultOutsideTemp)
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

    -- Recover outside temperature towards ambient temperature.
    local outsideTemp = pb:GetOutsideTemp()

    local outsideDifference = defaultOutsideTemp - outsideTemp

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
    local droneTemp = pb:GetDroneTemp()
    local freezeResistance = pb:GetFreezeResistance()

    if currentFreeze > 0 then
        -- Calculate how far above the freeze threshold the drone is.
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

    -- Update movement speed.
    currentFreeze = pb:GetFreezeLevel()

    local freezePercent = math.min(
        1.0,
        currentFreeze / freezeConfig.maxFreeze
    )

    local speedMultiplier = 1.0 -
        freezePercent * (1.0 - freezeConfig.minSpeedMultiplier)

    MovementModifiers.SetSlow(speedMultiplier)
end

function RenderUI()
    tracy.ZoneBeginN("FreezeController RenderUI")

    if pb ~= nil then
        local outsideTemp = pb:GetOutsideTemp()
        local droneTemp = pb:GetDroneTemp()
        local freezeResistance = pb:GetFreezeResistance()
        local freezeLevel = pb:GetFreezeLevel()

        local freezePercent = math.min(
            100.0,
            (freezeLevel / freezeConfig.maxFreeze) * 100.0
        )

        ImGui.Spacing()
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
