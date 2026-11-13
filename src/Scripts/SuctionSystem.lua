SuctionSystem = SuctionSystem or {}
SuctionSystem.fishes = SuctionSystem.fishes or {}
SuctionSystem.upgrades = SuctionSystem.upgrades or {}

function OnStart()
end

function OnUpdate()
end

function RenderUI()
end

function SuctionSystem.RegisterCollectible(node, type)
    if not node or not type then
        return
    end

    local registry


    if type == "fish" then
        registry = SuctionSystem.fishes
    else
        registry = SuctionSystem.upgrades
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

function SuctionSystem.UnregisterCollectible(node, type)
    if not node then
        return
    end
    
    local registry

    if type == "fish" then
        registry = SuctionSystem.fishes
    else
        registry = SuctionSystem.upgrades
    end
    
    for i = #registry, 1, -1 do
        if registry[i].node == node then
            table.remove(registry, i)
            return
        end
    end

    print("Collectible was not registered")
end

function SuctionSystem.GetFishes()
    return SuctionSystem.fishes
end

function SuctionSystem.GetUpgrads()
    return SuctionSystem.upgrades
end

return SuctionSystem