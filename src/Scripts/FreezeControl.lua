local MovementModifiers = require("MovementModifiers")
-- Get the two shared variables of freeze
local freezeConfig = require("Freeze")

local pb = nil

local recoveryDelay = 3.0
local recoveryTimer = 0.0
local thawRate = 15.0

-- Called when script starts
function OnStart()
    local player = self:GetPlayerComponent()
    if player ~= nil then
        pb = player:GetPlayerBehaviour()
    end
    
end

-- Called every frame
function OnUpdate(dt)
    if pb == nil then
        return
    end
    
    local timeSinceFreeze = pb:GetFreezeTimer()
    currentFreeze = pb:GetFreezeLevel()
    -- Check if the player can recover
    -- There is a timer so that the player still feel the freeze
    -- even after leaving the trigger zone
    if timeSinceFreeze >= 0 then
        timeSinceFreeze = timeSinceFreeze - dt
        pb:SetFreezeTimer(timeSinceFreeze)
        return
    end
    
    local currentFreeze = pb:GetFreezeLevel()
    
    -- If freeze levels are already 0 return skip calculating warm up
    if currentFreeze <= 0 then
        return
    end
    
    local decrease = math.min(currentFreeze, thawRate * dt)
    pb:IncreaseFreezeLevel(-decrease)
    
    local freezePercent = math.min(1.0, currentFreeze / freezeConfig.maxFreeze)

    local speedMultiplier = 1.0 - freezePercent * (1.0 - freezeConfig.minSpeedMultiplier)

    MovementModifiers.SetSlow(speedMultiplier)
end

-- Called to render variables in the script
function RenderUI()

end
