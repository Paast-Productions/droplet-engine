local FishSystem = {}
FishSystem.fishes = {}

function OnStart()
end

function OnUpdate()
end

function FishSystem.RegisterFish(node)
    table.insert(FishSystem.fishes, node)
end

function FishSystem.GetFishes()
    return FishSystem.fishes
end

return FishSystem