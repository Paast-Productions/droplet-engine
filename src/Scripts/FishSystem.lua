local FishSystem = {}
FishSystem.fishes = {}

function OnStart()
end

function OnUpdate()
end

function RenderUI()
end

function FishSystem.RegisterFish(node)
    table.insert(FishSystem.fishes, node)
end

function FishSystem.UnregisterFish(node)
    if node == nil then
        return
    end
    

    for i, fish in ipairs(FishSystem.fishes) do
        if fish == node then
            table.remove(FishSystem.fishes, i)
            return
        end
    end

    print("Fish was not registered")
end

function FishSystem.GetFishes()
    return FishSystem.fishes
end

return FishSystem