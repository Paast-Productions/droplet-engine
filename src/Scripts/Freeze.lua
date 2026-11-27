
local MovementModifiers = require("MovementModifiers")

local damageComp = nil

-- Freeze configuration
local freezeConfig = {}
freezeConfig.maxFreeze = 1000.0
freezeConfig.minSpeedMultiplier = 0.2

local freezeRate = 50
local damageScaling = 5
local recoveryDelay = 3.0

-- Temperature configuration
local temperatureChangeRate = 2.0
local temperatureScaling = 0.05

function OnStart()
    damageComp = self:GetHazardComponent()

    if damageComp == nil then
        print("ERROR: DamageComponent missing!")
        return
    end
end

function OnTrigger(dt, node)
    local health = node:GetHealthComponent()
    local player = node:GetPlayerComponent()

    if health == nil or player == nil or damageComp == nil then
        return
    end

    local pb = player:GetPlayerBehaviour()

    if pb == nil then
        return
    end

    local droneTemp = pb:GetDroneTemp()
    local outsideTemp = -100 * damageComp:GetDamage()

    pb:SetOutsideTemp(outsideTemp)

    local freezeResistance = pb:GetFreezeResistance()
    local freezeThreshold = -freezeResistance

    -- Calculate how far outside temperature is below resistance.
    local outsideFreezeDifference = math.max(
        0.0,
        freezeThreshold - outsideTemp
    )

    -- Scale temperature decrease based on resistance difference.
    local temperatureMultiplier = 1.0 +
        outsideFreezeDifference * temperatureScaling

    local currentTemperatureRate =
        temperatureChangeRate * temperatureMultiplier

    -- Gradually move drone temperature towards outside temperature.
    local temperatureDifference = outsideTemp - droneTemp

    local temperatureChange = math.min(
        math.abs(temperatureDifference),
        currentTemperatureRate * dt
    )

    if temperatureDifference < 0.0 then
        droneTemp = droneTemp - temperatureChange
    else
        droneTemp = droneTemp + temperatureChange
    end

    pb:SetDroneTemp(droneTemp)

    -- Calculate how far below freeze resistance the drone is.
    local freezeDifference = math.max(
        0.0,
        freezeThreshold - droneTemp
    )

    if freezeDifference > 0.0 then

        -- Normalize temperature difference.
        local freezeSeverity = math.min(freezeDifference / 50.0, 1.0)

        -- Increase freeze level.
        local freezeIncrease = freezeRate * freezeSeverity * dt
        pb:IncreaseFreezeLevel(freezeIncrease)

        local currentFreeze = math.min(
            pb:GetFreezeLevel(),
            freezeConfig.maxFreeze
        )

        -- Apply progressive movement slowdown.
        local freezePercent = currentFreeze / freezeConfig.maxFreeze

        local speedMultiplier = 1.0 -
            freezePercent * (1.0 - freezeConfig.minSpeedMultiplier)

        MovementModifiers.SetSlow(speedMultiplier)

        -- Apply progressively increasing damage.
        local damagePerSecond = 1.0 +
            damageScaling * freezeSeverity * freezeSeverity

        health:TakeDamage(damagePerSecond * dt)
    end

    -- Prevent immediate recovery after leaving the hazard.
    pb:SetFreezeTimer(recoveryDelay)
end

function RenderUI()
end

return freezeConfig
