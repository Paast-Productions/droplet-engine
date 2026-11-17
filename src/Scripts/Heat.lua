
local hazardComp = nil

-- Damage per second.
local damageNormal = 20.0
local damageResistant = 10.0

function OnStart()
end

function OnTrigger(dt, node)
    tracy.ZoneBeginN("HeatHazard OnTrigger")

    if node == nil then
        tracy.ZoneEnd()
        return
    end

    local health = node:GetHealthComponent()
    local player = node:GetPlayerComponent()

    if health == nil or player == nil then
        tracy.ZoneEnd()
        return
    end

    local pb = player:GetPlayerBehaviour()

    if pb == nil then
        tracy.ZoneEnd()
        return
    end

    -- Increase heat while inside the hazard.
    pb:SetHeatLevel(100)

    -- Determine damage based on heat resistance.
    local damagePerSecond = damageNormal

    if pb:GetHeatResistance() >= 100 then
        damagePerSecond = damageResistant
    end

    -- Apply frame-rate independent damage.
    health:TakeDamage(damagePerSecond * dt)

    tracy.ZoneEnd()
end

function RenderUI()
end
