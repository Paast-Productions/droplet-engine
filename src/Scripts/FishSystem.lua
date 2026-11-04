local FishSystem = {}
FishSystem.fish = {}

function OnStart()
end

function OnUpdate()
end

function FishSystem.RegisterFish(node)
    table.insert(FishSystem.fish, node)
    print("FishSystem called wowowowo")
end

function FishSystem.GetFishes()
    return FishSystem.fish
end

return FishSystem