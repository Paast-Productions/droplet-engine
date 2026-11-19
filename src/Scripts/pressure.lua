local MovementModifiers = require("MovementModifiers")
local damageHull = nil
local damageHealth = nil
local baseDamage = nil
local haz = nil

local PressureZones =
{
    {pressureLevel = 1, gravityMultiplier = 1.0, damageMultiplierForHull = 1.0, damageMultiplierForHealth = 1.5},
    {pressureLevel = 2, gravityMultiplier = 2.0, damageMultiplierForHull = 1.5, damageMultiplierForHealth = 2.0},
    {pressureLevel = 3, gravityMultiplier = 3.0, damageMultiplierForHull = 2.0, damageMultiplierForHealth = 2.5},
    {pressureLevel = 4, gravityMultiplier = 4.0, damageMultiplierForHull = 2.5, damageMultiplierForHealth = 3.0},
    {pressureLevel = 5, gravityMultiplier = 5.0, damageMultiplierForHull = 3.0, damageMultiplierForHealth = 3.5}
}

local higher = false

function CheckPressureZone(zone)
    for x = 1, #PressureZones do 
        if zone == PressureZones[x].pressureLevel then
            MovementModifiers.SetGravityPull(PressureZones[x].gravityMultiplier)
            damageHull = PressureZones[x].damageMultiplierForHull + baseDamage * 0.25
            damageHealth = PressureZones[x].damageMultiplierForHealth + baseDamage * 0.25
            break
        end
    end
end

function OnStart()
    print("Pressure")

    haz = self:GetHazardComponent()
    baseDamage = haz:GetDamage()
end

function OnTrigger(dt, node)
    local trans = node:GetTransform() 
    --see if i can remove this one
    -- local health = node:GetHealthComponent()
    local playerComponent = node:GetPlayerComponent()
    if playerComponent == nil then
        print("playercomp fel") 
        return
    end

    local pb = playerComponent:GetPlayerBehaviour()
    if pb == nil then
        print("pb fel")
        return
    end
    local gravityMultiplier = MovementModifiers.GetGravityPull()
    local hullIntegrity = pb:GetHullIntegrity()
    local zone = haz:GetZone()
    local getCurrentLevel = pb:GetPressureLevel()
    local health = node:GetHealthComponent()

    if zone == getCurrentLevel then
        CheckPressureZone(zone)
        higher = false
    elseif zone > getCurrentLevel then
        CheckPressureZone(zone)
        higher = true
    else
        CheckPressureZone(zone)
        higher = false
    end

    CheckPressureZone(zone)
    
    local givinDamage = damageHull * dt
    health:TakeDamageHullIntergrity(givinDamage)

    if higher then
        local givinDamage = damageHealth * dt
        health:TakeDamage(givinDamage)
    end

    MovementModifiers.SetGravityPull(1)


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
