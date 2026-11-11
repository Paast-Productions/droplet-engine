local MovementModifiers = require("MovementModifiers")
local presureDamage = nil
--local player = nil

-- this one is for the player gets damage each second
local lastTimeGotDamage = 1

local PressureZones = {}

function OnStart()
    print("Pressure")
    -- local GetPlayerComponent = GetPlayerComponent()

    local damage = self:GetDamageComponent()
    presureDamage = (damage:GetDamage()) * 0.25
end

-- Called every frame
function OnUpdate(dt)
end

function OnTrigger(dt, node)
    local trans = node:GetTransform() 
    local health = node:GetHealthComponent()
    --local playerComponent = node:GetPlayerBehaviour()

    local gravityMultiplier = MovementModifiers.GetGravityPull()

    -- player = playerComponent:GetPlayerBehaviour()
    -- local pressureLevelResistance = player:GetPressureLevel()
    -- local pressurResistance = player:GetPressureResistance()

    -- print(pressureLevelResistance .. " <-pressure resistance")
    -- print(pressurResistance .. " <-pressure resistance")

    lastTimeGotDamage = lastTimeGotDamage + dt
    if lastTimeGotDamage > 1.0 then 
       lastTimeGotDamage = 0
        print(lastTimeGotDamage .. " <- <- damage last time")

        print("hello world")
        local givinDamage = presureDamage
        print(givinDamage .. " <- damage")


        local positionOfDrone = trans:GetPosition()

        health = node:GetHealthComponent()
        health:TakeDamage(givinDamage)
        local Ghealth = health:GetHealth()
        print(Ghealth)
    end

    local dronePos = trans:GetPosition()
    -- 0.1 is the strength of gravity, making the drone easy to control
    dronePos.y = dronePos.y + (gravityMultiplier * 0.1)
    trans:SetPosition(dronePos)
end

function RenderUI()

end