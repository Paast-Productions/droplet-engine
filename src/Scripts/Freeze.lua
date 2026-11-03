local MovementModifiers = require("MovementModifiers")

local damage = 10.0
-- Called when script starts
function OnStart()
    
end

-- Called every frame
function OnUpdate(dt)

end

function OnTrigger(dt, node)
    local health = node:GetHealthComponent()

    if not health then
        return
    end
    
    health:TakeDamage(damage * dt)
    print("player health", health:GetHealth())

    MovementModifiers.ApplyFreeze()

end

-- Called to render variables in the script
function RenderUI()

end
