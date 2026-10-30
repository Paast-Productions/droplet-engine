local itemToCraft = "woodfish"
local amountOfFish = 10
local amountOfMaterial
local trans = nil
--local interactionRadius = 3.0 

--function DroneIsNear()
    --DOTO INCLUDE PLAYER INSTEAD OF CUBEPOS
    --local cubePos = helper.cubePos()

    --local TablePos = cubeTrans:GetPosition(TransformSpace.World)
    --local vectorToTable = GLM.V3Sub(TablePos, cubePos)
    --local distance = GLM.V3Length(vectorToTable)
    
    --ImGui.Text("Table distance: " .. string.format("%.2f", distance))

    --if distance <= interactionRadius then
        --print("cube is near!")
        
        --if Input:KeyPressed(Key.E) then
            --print("interact with table!")
        --end
    --end
--end
function Upgrade()
    local priceOfItem = 10;
    -- Example of how to handle crafting logic
    -- DOTO CHANGE THE PRICE FOR EACH ITEM
    if(itemToCraft and priceOfItem <= amountOfMaterial) then
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
    print("Interacted witer:")
    -- Example of how to handle crafting logic
    -- DOTO CHANGE THE PRICE FOR EACH ITEM
    local priceOfItem = 2;
    if(itemToCraft and priceOfItem <= amountOfFish) then
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
end

function RenderUI()
    -- Render the UI for the crafting table
    if Input:KeyPressed(Key.E) then
        Crafting()
    end

    if trans then 
        local tablePos = trans:GetPosition(TransformSpace.World)
        ImGui.Text("Table Pos: " .. string.format("%.2f, %.2f, %.2f", tablePos.x, tablePos.y, tablePos.z))
    end
end
