
local MovementModifiers = {}

MovementModifiers.speedMultiplier = 1.0
MovementModifiers.handlingMultiplier = 1.0
MovementModifiers.gravityMultiplier = 1.0

function MovementModifiers.SetSlow(multiplier)
    MovementModifiers.speedMultiplier =
        math.max(0.0, math.min(1.0, multiplier))
end

function MovementModifiers.ClearSlow()
    MovementModifiers.speedMultiplier = 1.0
end

function MovementModifiers.GetSpeedMultiplier()
    return MovementModifiers.speedMultiplier
end

function MovementModifiers.SetHandling(multiplier)
    MovementModifiers.handlingMultiplier =
        math.max(0.0, math.min(1.0, multiplier))
end

function MovementModifiers.GetHandling()
    return MovementModifiers.handlingMultiplier
end

function MovementModifiers.SetGravityPull(multiplier)
    MovementModifiers.gravityMultiplier = multiplier
end

function MovementModifiers.ClearGravityPull()
    MovementModifiers.gravityMultiplier = 1.0
end

function MovementModifiers.GetGravityPull()
    return MovementModifiers.gravityMultiplier
end

return MovementModifiers
