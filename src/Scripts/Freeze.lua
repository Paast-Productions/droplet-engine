local MovementModifiers = require("MovementModifiers")

local damageComp = nil
local freezeDamage = nil
-- Called when script starts
function OnStart()
    damageComp = self:GetDamageComponent()
        print("DamageComponent valid:", damageComp ~= nil)
    freezeDamage = damageComp:GetDamage()
end

-- Called every frame
function OnUpdate(dt)

end

function OnTrigger(dt, node)
    local health = node:GetHealthComponent()
    local player = node:GetPlayerComponent()
    local pb = nil
    if player ~= nil then
        pb = player:GetPlayerBehaviour()
    end
    print("Health valid:", health ~= nil)
    print("PlayerBehaviour valid:", pb ~= nil)
    print("DamageComponent valid:", damageComp ~= nil)
    if health ~= nil and pb ~= nil and damageComp ~= nil then
        pb:IncreaseFreezeLevel(1)
        print("freezeLevel", pb:GetFreezeLevel())        
        if pb:GetFreezeLevel() > pb:GetFreezeResistance() then
            health:TakeDamage(freezeDamage * dt)
        end
    end
    -- Apply Damage
    --health:TakeDamage(freezeDamage * dt)
    print("player health", health:GetHealth())

    -- Notify the movementmodifer to apply slow
    MovementModifiers.ApplyFreeze()

end

-- Called to render variables in the script
function RenderUI()
    
end
