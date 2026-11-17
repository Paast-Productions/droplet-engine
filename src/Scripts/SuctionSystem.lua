SuctionSystem = SuctionSystem or {}
SuctionSystem.fishes = SuctionSystem.fishes or {}
SuctionSystem.upgrades = SuctionSystem.upgrades or {}
SuctionSystem.brokenParts = SuctionSystem.brokenParts or {}

function OnStart()
end

function OnUpdate()
end

function RenderUI()
end

function SuctionSystem.RegisterCollectible(node)
    local comp = node:GetCollectibleTypeComponent()

    if not node or not comp then
        return
    end

    local registry
    local type = comp:GetCollectibleType()
    if type == CollectibleType.Fish then
        registry = SuctionSystem.fishes 
    elseif type == CollectibleType.Upgrade then
        registry = SuctionSystem.upgrades
    elseif type == CollectibleType.BrokenPart then
        registry = SuctionSystem.brokenParts
    else
        print("Type of component is not recogised")
    end

    for _, entry in ipairs(registry) do
        if entry.node == node then
            return
        end
    end

    local item = comp:GetItemId()
    local amount = comp:GetAmount()

    if not item or not amount then
        print("No defined item or amount for collectible")
        return
    end

    table.insert(registry, 
    {
        node = node,
        type = type,
        item = item,
        amount = amount
    })
end

function SuctionSystem.UnregisterCollectible(node)
    local comp = node:GetCollectibleTypeComponent()
    if not comp then
        return
    end
    
    local registry
    local type = comp:GetCollectibleType()

    if type == CollectibleType.Fish then
        registry = SuctionSystem.fishes
    elseif type == CollectibleType.Upgrade then
        registry = SuctionSystem.upgrades
    elseif type == CollectibleType.BrokenPart then
        registry = SuctionSystem.brokenParts
    else
        print("Register doesnt exits")
        return
    end
    
    for i = #registry, 1, -1 do
        if registry[i].node == node then
            print("Remove", node.GetName)
            table.remove(registry, i)
            return
        end
    end

    print("Collectible was not registered", node:GetName())
end

function SuctionSystem.GetFishes()
    return SuctionSystem.fishes
end

function SuctionSystem.GetUpgrads()
    return SuctionSystem.upgrades
end

function SuctionSystem.GetBrokenParts()
    return SuctionSystem.brokenParts
end

return SuctionSystem