FishSystem = FishSystem or {}
FishSystem.fishes = FishSystem.fishes or {}
FishSystem.upgrades = FishSystem.upgrades or {}

function OnStart()
end

function OnUpdate()
end

function RenderUI()
end

function FishSystem.RegisterFish(node, type)
    if not node or not type then
        return
    end

    local registry


    if type == "fish" then
        registry = FishSystem.fishes
    else
        registry = FishSystem.upgrades
    end

    for _, entry in ipairs(registry) do
        if entry.node == node then
            return
        end
    end

    table.insert(registry, 
    {
        node = node,
        type = type
    })
end

function FishSystem.UnregisterFish(node, type)
    if not node then
        return
    end
    
    local registry

    if type == "fish" then
        registry = FishSystem.fishes
    else
        registry = FishSystem.upgrades
    end
    
    for i = #registry, 1, -1 do
        if registry[i].node == node then
            table.remove(registry, i)
            return
        end
    end

    print("Collectible was not registered")
end

function FishSystem.GetFishes()
    return FishSystem.fishes
end

function FishSystem.GetUpgrads()
    return FishSystem.upgrades
end

return FishSystem