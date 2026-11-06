
-- Called when script starts
function OnStart()

end

-- Called every frame
function OnUpdate(dt)

end

function OnTrigger(dt, node)
    local health = node:GetHealthComponent()

    if health then
    print("player health", health:GetHealth())
    health:TakeDamage(1)
    end
end

-- Called to render variables in the script
function RenderUI()

end
