local MovementModifiers = {}

MovementModifiers.freeze = 0.0
MovementModifiers.minSpeedModifier = 0.27
MovementModifiers.triggerTimer = 0.0

local freezeRate = 0.3
local recoveryRate = 0.015

-- Called when script starts
function OnStart()

end

-- Called every frame
function OnUpdate(dt)

end

-- Called to render variables in the script
function RenderUI()

end

function MovementModifiers.ApplyFreeze()
    --MovementModifiers.freeze = math.minI(1.0, MovementModifiers.freeze + amount)
    MovementModifiers.triggerTimer = 0.1
    end

function MovementModifiers.Update(dt)
    if MovementModifiers.triggerTimer > 0 then
        MovementModifiers.freeze = 
            math.min(1.0, MovementModifiers.freeze + freezeRate * dt)
    else
        MovementModifiers.freeze = 
            math.max(0.0, MovementModifiers.freeze - recoveryRate * dt)
    --MovementModifiers.freeze = math.max(0.0, MovementModifiers - 0.2 * dt)
    end

    MovementModifiers.triggerTimer = 
        math.max(0.0, MovementModifiers.triggerTimer - dt)

end

function MovementModifiers.GetSpeedMultiplier()
    return 1.0 - MovementModifiers.freeze * (1.0 - MovementModifiers.minSpeedModifier)
end

return MovementModifiers