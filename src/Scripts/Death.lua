local state = "ALIVE"

local deathTimer = 0.0
local deathDuration = 3.0
local sinkSpeed = 0.4

local screenAlpha = 0.0

local health = nil
local physics = nil
local trans = nil
local pb = nil

local movementScript = nil

-- Called when script starts
function OnStart()
    health = self:GetHealthComponent()
    physics = self:GetPhysicsComponent()
    trans = self:GetTransform()

    local player = self:GetPlayerComponent()
    pb = player:GetPlayerBehaviour()
    movementScript = self:GetScriptComponent("DroneControl.lua")

    print("MovementScript: ", movementScript)

    if movementScript == nil then
        print("ERROR: Can find movement script")
    end
end

-- Called every frame
function OnUpdate(dt)

    if health == nil or physics == nil or trans == nil then
        return
    end
    
    if state == "ALIVE" then
        if health:GetHealth() <= 0 then
            state = "DYING"
            deathTimer = 0.0
            movementScript:SetActiveSelf(false)
        end

    elseif state == "DYING" then
        deathTimer = math.min(
            deathTimer + dt, 
            deathDuration
        )

        local progress = deathTimer / deathDuration

        screenAlpha = progress * progress * (3.0 - 2.0 * progress)

        physics:SetVelocity(Vec3.new(0.0, -sinkSpeed * dt, 0.0))

        if deathTimer >= deathDuration then
            state = "RESPAWNING"
        end
    elseif state == "RESPAWNING" then
        local newPos = Vec3.new(46.000, 12.000, -20.000)
        trans:SetPosition(newPos)
        health:Heal(health:GetMaxHealth())

        pb:ClearDroneInventory()

        deathTimer = 0.0
        state = "ALIVE"
        screenAlpha = 0.0
        movementScript:SetActiveSelf(true)
    end
end

-- Called to render variables in the script
function RenderUI()

end
