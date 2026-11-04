local damageComp = nil
local heatDamage = nil

function OnStart()
    damageComp = self:GetDamageComponent()
    
    heatDamage = damageComp:GetDamage()
end

function OnTrigger(dt, node)
    tracy.ZoneBeginN("HeatHazard OnTrigger")
    
    local health = node:GetHealthComponent()
    local player = node:GetPlayerComponent()
    local pb = nil
    if player ~= nil then
        pb = player:GetPlayerBehaviour()
    end
    
    if damageComp ~= nil and pb ~= nil and health ~= nil then

        pb:IncreaseHeatLevel(1)

        if pb:GetHeatLevel() > pb:GetHeatResistance() then
            health:TakeDamage(heatDamage * dt)

            if health:GetHealth() <= 0 then
            end
        end
    end

    tracy.ZoneEnd()
end

function RenderUI()
    
end