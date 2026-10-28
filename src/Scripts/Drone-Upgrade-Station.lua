local itemToCraft = "fish"
local amountOfFish = 10 

function OnInteract()
    print("Interacted witer:")

    -- Example of how to handle crafting logic
    local priceOfItem = 2;
    if(itemToCraft and priceOfItem <= amountOfFish) then
        -- Check if the player has the required items for crafting
        -- If they do, perform the crafting logic
        newItem = "pink light"
        print("Crafting " .. newItem .. "")
        amountOfFish = amountOfFish - priceOfItem
        print(amountOfFish)
    else
        print("Not enough resources to craft " .. itemToCraft)
    end
end

function OnStart()
    print("Drone Upgrade Station")
end

-- Called every frame
function OnUpdate(dt)
    print("botato")
end

function RenderUI()
    -- Render the UI for the crafting table
    if Input:KeyPressed(Key.E) then
        OnInteract()
        -- Here you would implement the actual UI rendering logic
    end
end
