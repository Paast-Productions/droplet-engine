
--make the math easier
local unique = nil
local itemRegistry = nil
local player = nil

local amount = 0

-- variabels
local touchedTheMesh = false
local word
-- variabels that is need to gets code from c++¨

function OnStart()
    print("Drone Upgrade Station")
end

function Crafting(id, name)
    print("Crafting station:")

    local itemId = itemRegistry:Find(id)
    amount = player:CountItem(InventoryType.Player, itemId)
    print("amount material-> " .. amount)


    -- Example of how to handle crafting logic
    -- DOTO CHANGE THE PRICE FOR EACH ITEM
    local priceOfCraft = 2;
    if amount >= priceOfCraft then
        amount = amount - priceOfCraft
        print(amount .. " <- amount of fish got removed by small calulation")
        -- Create the crafted item and add it to the player's inventory for the drone
        -- speed, acceleration, drag 

        --droplet-engine/    droplet-engine/src/Scripts/Engine_Side and DroneMovement.lua
        --
        local succes = player:RemoveItem(InventoryType.Player, itemId, priceOfCraft)
        if succes then 
            print("Succes")            
        end
    else
        print("Not enough resources to craft " .. name)
    end
end


function RenderUI()
    -- DOTO
    -- Currently im using Imgui
    -- The remaining indices need to be selected through the 2D engine.

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

function OnTrigger(dt, node)
    --print("Drigger works")
    touchedTheMesh = true
    local playerComponent = node:GetPlayerComponent()

    player = playerComponent:GetPlayerBehaviour()
    unique = player:GetUniqueID()
    itemRegistry = player:GetItemRegistry() 
end

function OnUpdate()
    if touchedTheMesh then
        -- DOTO
        -- Currently im using Imgui
        -- The remaining indices need to be selected through the 2D engine.
        if Input:KeyPressed(Key.E) then
            if "wood" == word then
                print("wood")
                Crafting("resource:wood", "Wood")
            end

            if "stone" == word then
                print("stone")
                Crafting("resource:stone", "Stone")
            end 
    
            if "iron" == word then
                print("iron")
                Crafting("resource:iron", "Iron")
            end
            touchedTheMesh = false
        end
    end
end