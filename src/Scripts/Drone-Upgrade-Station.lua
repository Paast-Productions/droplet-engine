local itemToCraft = "woodfish"
local amountOfFish = 10
local amountOfMaterial
local trans = nil
local touchedTheMesh = false
local player = nil

function Upgrade()
    print("Material station:")
    local priceOfItem = 10;
    -- Example of how to handle crafting logic
    -- DOTO CHANGE THE PRICE FOR EACH ITEM
    if(priceOfItem <= amountOfMaterial) then
        -- Check if the player has the required items for crafting
        -- If they do, perform the crafting logic
        newItem = "pink light"
        print("upgrading light " .. newItem .. "")
        amountOfMaterial = amountOfMaterial - priceOfItem
        print(amountOfMaterial)
    else
        print("Not enough resources to craft " .. itemToCraft)
    end
end

function Crafting()
    print("Crafting station:")
    -- Example of how to handle crafting logic
    -- DOTO CHANGE THE PRICE FOR EACH ITEM
    local priceOfItem = 2;
    if(priceOfItem <= amountOfFish) then
        -- Check if the player has the required items for crafting
        -- If they do, perform the crafting logic
        newItem = "wood"
        print("Crafting " .. newItem .. "")
        amountOfFish = amountOfFish - priceOfItem
        print(amountOfFish)
    else
        print("Not enough resources to craft " .. itemToCraft)
    end
end
 
function OnStart()
    print("Drone Upgrade Station")
    trans = self:GetTransform()
    player = self:GetPlayerData()

    if player then
        local size = player:InventorySize()
        for i = 0, size - 1 do
            local ItemId = player:InventoryItemId(i)
            local count = player:InventoryCount(i)
            print("ItemId [" .. i .. "]: " .. ItemId)
            print("count [" .. i .. "]: " .. tostring(count))
        end
    end
end

function RenderUI()
end

function OnTrigger()
    touchedTheMesh = true
end

function OnUpdate()
    if touchedTheMesh then
        if Input:KeyPressed(Key.E) then
            Crafting()
            touchedTheMesh = false
        end
    end
end
