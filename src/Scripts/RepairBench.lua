local KeyPressed = false
-- Called when script starts
function OnStart()

end

-- Called every frame
function OnUpdate(dt)
    if Input:KeyPressed(Key.E) then
        if (KeyPressed == false) then
            KeyPressed = true
        end
    end
end

function OnTrigger(dt, node)
    tracy.ZoneBeginN("RepairBench OnTrigger")
    local player = node:GetPlayerComponent()
    local pb = nil
    if player ~= nil then
        pb = player:GetPlayerBehaviour()
        local registry = pb:GetItemRegistry()
        if registry == nil then
            print("Item registry is unavailable!")
        else
            BrokenPartId = registry:Find("upgrade:broken_engine_part")
            RepairedPartId = registry:Find("upgrade:repaired_engine_part")
            WoodId = registry:Find("resource:wood")
            
            if KeyPressed == true and pb:CountItem(InventoryType.Player, BrokenPartId) == 1 then
                if pb:CountItem(InventoryType.Player, WoodId) >= 5 then
                    print("used 5 wood and repaired engine part")
                    pb:RemoveItem(InventoryType.Player, WoodId, 5)
                    pb:RemoveItem(InventoryType.Player, BrokenPartId, 1)
                    pb:AddItem(InventoryType.Player, RepairedPartId, 1)
                else
                    print("Not enough wood ")
                end
                print("")
                KeyPressed = false
            else
                if KeyPressed == true then
                print("cant do nathan")
                end
            end
        end
    end
    tracy.ZoneEnd()
end

-- Called to render variables in the script
function RenderUI()

end
