
--make the math easier
local unique = nil
local itemRegistry = nil
local amount = 0

-- bool
local touchedTheMesh = false

-- variabels that is need to gets code from c++¨
local player = nil

function OnStart()
    print("Drone Upgrade Station")
    local playerComponent = self:GetPlayerComponent()

    player = playerComponent:GetPlayerBehaviour()
    unique = player:GetUniqueID()
    itemRegistry = player:GetItemRegistry() 
end

function Crafting()
    print("Crafting station:")
    -- Example of how to handle crafting logic
    -- DOTO CHANGE THE PRICE FOR EACH ITEM
    local priceOfCraft = 2;
    if amount >= priceOfCraft then
        amount = amount - priceOfCraft
        print(amount .. " <- amount of fish got removed by small calulation")
        
        
        local succes = player:RemoveItem(InventoryType.Player, unique[x], amount)
        if succes then 
            print(succes .. " <- removeing item is")            
        end
    else
        print("Not enough resources to craft " .. itemToCraft)
    end
end


function RenderUI()
end

function OnTrigger()

    touchedTheMesh = true
end

function OnUpdate()
    -- DOTO
    -- Currently hardcoded. The first index is processed automatically.
    -- The remaining indices need to be selected through the 2D engine or its components.
    amount = player:CountItem(InventoryType.Player, unique[1])
    print("amount-> " .. amount)

    if touchedTheMesh then
        if Input:KeyPressed(Key.E) then
            Crafting()
            touchedTheMesh = false
        end
    end
end