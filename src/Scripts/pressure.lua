local MovementModifiers = require("MovementModifiers")
local presureDamage = nil

-- this one is for the player gets damage each second
local lastTimeGotDamage = 1

function OnStart()
    print("Pressure")
    -- local GetPlayerComponent = GetPlayerComponent()
    -- player = GetPlayerComponent:GetPlayerBehaviour()
    local damage = self:GetDamageComponent()
    if damage == nil then
        print("There is no damage componenet")
        return
    end
    presureDamage = (damage:GetDamage()) * 0.25
    if presureDamage == nil then
        print("There is no presureDamage")
        return
    end
    print(presureDamages)    
end

-- Called every frame
function OnUpdate(dt)
end

function OnTrigger(dt, node)
    local trans = node:GetTransform() 
    local health = node:GetHealthComponent()

    local gravityMultiplier = MovementModifiers.GetGravityPull()

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
