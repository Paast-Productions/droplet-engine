local MovementModifiers = require("MovementModifiers")

local hazardComp = nil
local freezeDamage = 0

-- Freeze configuration
local freezeRate = 30.0
local freezeThreshold = 200.0
local freezeTier = 1

-- Slow configuration
-- These two variables are shared between this script and freezecontroller
local freezeConfig = {}
freezeConfig.maxFreeze = 1000.0
freezeConfig.minSpeedMultiplier = 0.2

-- Damage configuration
local damageScaling = 0.65

function OnStart()
    hazardComp = self:GetHazardComponent()
        if hazardComp == nil then
            print("ERROR: HazardComponent missing!")
            return
        end
    freezeDamage = (hazardComp:GetDamage()) * 0.25
end

-- This trigger function slows down and damages the player
-- progressevly worse
function OnTrigger(dt, node)
    local health = node:GetHealthComponent()
    local player = node:GetPlayerComponent()

    if health == nil or player == nil or hazardComp == nil then
        return
    end

    local pb = player:GetPlayerBehaviour()

    if pb == nil then
        return
    end

    -- Increase the freeze level
    local currentFreeze = pb:GetFreezeLevel()

    local freezeIncrease = freezeRate * dt
    
    if currentFreeze < freezeConfig.maxFreeze then
        pb:IncreaseFreezeLevel(freezeIncrease * (0.5 * freezeTier)^2)
    end

    -- Apply slow
    local freezePercent = math.min(1.0, math.max(0.0, currentFreeze / freezeConfig.maxFreeze))

    local speedMultiplier = 1.0 - freezePercent * (1.0 - freezeConfig.minSpeedMultiplier)

    MovementModifiers.SetSlow(speedMultiplier)

    -- Get new tier that will exponentially damage the player more
    freezeTier = math.floor(currentFreeze / freezeThreshold) + 1

    currentFreeze = pb:GetFreezeLevel()

    if currentFreeze > pb:GetFreezeResistance() then
        local damagePerSecond = freezeDamage * (freezeDamage + (freezeTier * damageScaling)^2)
        health:TakeDamage(damagePerSecond * dt)
    end
    -- This timer prevents the palyer to recover immedietly after leaving the zone
    pb:SetFreezeTimer(3.0)
end

-- Called to render variables in the script
function RenderUI()
    
end

return freezeConfig
