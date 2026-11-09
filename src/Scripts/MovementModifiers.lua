local MovementModifiers = {}

MovementModifiers.speedMultiplier = 1.0

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

return MovementModifiers