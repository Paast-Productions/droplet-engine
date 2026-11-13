local MovementModifiers = require("MovementModifiers")
local presureDamage = nil
local baseDamage = nil
local haz = nil
local PressureRate = 50
local pressureThreshold = 200.0

pressureTier = 1.0
damageScaling = 0.1

local PressureZones =
{
    {pressureLevel = 1, gravityMultiplier = 1.0, damageMultiplier = 1.0},
    {pressureLevel = 2, gravityMultiplier = 2.0, damageMultiplier = 1.5},
    {pressureLevel = 3, gravityMultiplier = 3.0, damageMultiplier = 2.0},
    {pressureLevel = 4, gravityMultiplier = 4.0, damageMultiplier = 2.5},
    {pressureLevel = 5, gravityMultiplier = 5.0, damageMultiplier = 3.0}
}

function CheckPressureZone(pressureLevelResistance)
    for x = 1, #PressureZones do 
        if pressureLevelResistance == PressureZones[x].pressureLevel then
            MovementModifiers.SetGravityPull(PressureZones[x].gravityMultiplier)
            presureDamage = PressureZones[x].damageMultiplier + baseDamage * 0.25
            break
        end
    end
end

function OnStart()
    print("Pressure")

    haz = self:GetHazardComponent()
    baseDamage = haz:GetDamage()
    presureDamage = baseDamage * 0.25
end

function OnTrigger(dt, node)
    local trans = node:GetTransform() 
    local health = node:GetHealthComponent()
    local playerComponent = node:GetPlayerComponent()
    local player = playerComponent:GetPlayerBehaviour()
    local gravityMultiplier = MovementModifiers.GetGravityPull()
    local zone = haz:GetZone()

    print(zone .. " <- zone")
    local getCurrentLevel = player:GetPressureLevel()
    local pressureIncrease = PressureRate * dt
    
    if getCurrentLevel < 1000 then
        player:IncreasePressureLevel(pressureIncrease * (0.5 * pressureTier)^2)
    end

    pressureTier = math.floor(getCurrentLevel / pressureThreshold) + 1
    print(getCurrentLevel .. " <- currentlevel")
    print(pressureThreshold .. " <- PressureThreshold")
    print(pressureTier .. "<- tier")

    if getCurrentLevel > player:GetPressureResistance() then 
        local givinDamage = presureDamage * (presureDamage + (pressureTier * damageScaling))
        print(givinDamage .. " <- damage")
    
        health = node:GetHealthComponent()
        health:TakeDamage(givinDamage)
        local Ghealth = health:GetHealth()
        print(Ghealth .. " <-Health for the player")
    end

    local dronePos = trans:GetPosition()
    -- 0.1 is the strength of gravity, making the drone easy to control
    dronePos.y = dronePos.y - (gravityMultiplier * 0.1 * dt)

    local shakeIntensity = gravityMultiplier * 0.05
    dronePos.x = dronePos.x + (math.random() - 0.5) * shakeIntensity
    dronePos.z = dronePos.z + (math.random() - 0.5) * shakeIntensity

    trans:SetPosition(dronePos)

end

function RenderUI()
end


--
-- spelar 2
-- om spalaren befinner sig i samma level som zonen armon inte skada och bara spealaren

-- spelaren 1  
--level zonen 2 mycket skada + armor skad

-- Hull integrity == armor
-- get och set


