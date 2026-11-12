local MovementModifiers = require("MovementModifiers")
local presureDamage = nil
local baseDamage = 0
--local player = nil

-- this one is for the player gets damage each second
local lastTimeGotDamage = 1

--This may need to be changed depending on whether the group or player designer decides otherwise.
local PressureZones =
{
    {pressureLevel = 1, gravityMultiplier = 1.0, damageMultiplier = 1.0},
    {pressureLevel = 2, gravityMultiplier = 2.0, damageMultiplier = 1.5},
    {pressureLevel = 3, gravityMultiplier = 3.0, damageMultiplier = 2.0},
    {pressureLevel = 4, gravityMultiplier = 4.0, damageMultiplier = 2.5},
    {pressureLevel = 5, gravityMultiplier = 5.0, damageMultiplier = 3.0}
}
local lastApplied = 0


function OnStart()
    print("Pressure")

    local haz = self:GetHazardComponent()
    baseDamage = haz:GetDamage()
    presureDamage = baseDamage * 0.25
end

function CheckPressureZone(pressureLevelResistance)
    for x = 1, #PressureZones do 
        if pressureLevelResistance == PressureZones[x].pressureLevel then
            MovementModifiers.SetGravityPull(PressureZones[x].gravityMultiplier)
            presureDamage = PressureZones[x].damageMultiplier + baseDamage * 0.25
            break
        end
    end
end


function OnTrigger(dt, node)
    local trans = node:GetTransform() 
    local health = node:GetHealthComponent()
    local playerComponent = node:GetPlayerComponent()
    local player = playerComponent:GetPlayerBehaviour()

    local gravityMultiplier = MovementModifiers.GetGravityPull()
    
    -- It is used to determine the pressure level required for the drone to access the area.
    -- TODO: Another component should be added to check the bounding box's in other word the node PressureLevel and other levels and determine whether the pressure is too high or safe for the player.
    local pressureLevelResistance = player:GetPressureLevel()
    if lastApplied ~= pressureLevelResistance then
        CheckPressureZone(pressureLevelResistance)
        lastApplied = pressureLevelResistance
    end
    

    lastTimeGotDamage = lastTimeGotDamage + dt
    if lastTimeGotDamage > 1.0 then 
       lastTimeGotDamage = 0

        local givinDamage = presureDamage
        print(givinDamage .. " <- damage")


        local positionOfDrone = trans:GetPosition()

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