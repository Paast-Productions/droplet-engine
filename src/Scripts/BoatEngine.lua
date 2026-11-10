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
    tracy.ZoneBeginN("BoatEngine OnTrigger")
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
            
            if KeyPressed == true and pb:CountItem(InventoryType.Player, BrokenPartId) == 1 and pb:CountItem(InventoryType.Player, RepairedPartId) == 0 then
                print("Cannot repair the engine with a broken part")
                KeyPressed = false
            else 
                if KeyPressed == true and pb:CountItem(InventoryType.Player, RepairedPartId) == 1 then
                    print("Repaired")
                    pb:RemoveItem(InventoryType.Player, RepairedPartId, 1)
                    KeyPressed = false
                end
            end
        end
    end
    tracy.ZoneEnd()
end

-- Called to render variables in the script
function RenderUI()

end
