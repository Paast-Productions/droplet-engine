
--make the math easier
local unique = nil
local itemRegistry = nil
local amount = 0

-- variabels
local touchedTheMesh = false
local word
-- variabels that is need to gets code from c++¨
local player = nil

function OnStart()
    print("Drone Upgrade Station")
    local playerComponent = self:GetPlayerComponent()

    player = playerComponent:GetPlayerBehaviour()
    unique = player:GetUniqueID()
    itemRegistry = player:GetItemRegistry() 
end

function Crafting(itemId)
    print("Crafting station:")
    print(itemId .. " <- word")
    local Id = itemRegistry:Find(itemId)
    amount = player:CountItem(InventoryType.Player, Id)
    print("amount wood-> " .. amount)

    -- Example of how to handle crafting logic
    -- DOTO CHANGE THE PRICE FOR EACH ITEM
    local priceOfCraft = 2;
    if amount >= priceOfCraft then
        amount = amount - priceOfCraft
        print(amount .. " <- amount of fish got removed by small calulation")
        

        local succes = player:RemoveItem(InventoryType.Player, Id, priceOfCraft)
        if succes then 
            print("Succes")            
        end
    else
        print("Not enough resources to craft " .. amount)
    end
end


function RenderUI()
    -- DOTO
    -- Currently hardcoded. The first index is processed automatically.
    -- The remaining indices need to be selected through the 2D engine or its components.

    if ImGui.Button("Wood") then
        word = "wood"
    end

    if ImGui.Button("Stone") then
        word = "stone"
    end 
    
    if ImGui.Button("Iron") then
        word = "iron"
    end
end

function OnTrigger()
    touchedTheMesh = true
end

function OnUpdate()
    --if touchedTheMesh then
        if Input:KeyPressed(Key.E) then
            if word == "wood" then
                Crafting("resource:wood")
            end
            if word == "stone" then
                Crafting("resource:stone")
            end
            if word == "iron" then
                Crafting("resource:iron")
            end
            touchedTheMesh = false
        end
    --end
end