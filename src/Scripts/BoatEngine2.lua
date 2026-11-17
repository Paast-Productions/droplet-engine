local InteractableSystem = require("InteractableSystem")
local MeshToggleSystem = require("MeshToggleSystem")

local brokenEngine
local repairedEngine

function OnStart()
    InteractableSystem.RegisterInteractable(self)

end

function OnDestroy()
    InteractableSystem.UnregisterInteractable(self)
end

function OnTrigger(dt, node)
    tracy.ZoneBeginN("BoatEngine OnTrigger")

    local player = node:GetPlayerComponent()

    if player == nil then
        tracy.ZoneEnd()
        return
    end

    local pb = player:GetPlayerBehaviour()

    if pb == nil then
        tracy.ZoneEnd()
        return
    end

    local registry = pb:GetItemRegistry()

    if registry == nil then
        print("Item registry is unavailable!")
        tracy.ZoneEnd()
        return
    end

    local repairedPartId = registry:Find("upgrade:repaired_engine_part")
    local woodId = registry:Find("resource:wood")
    local metalId = registry:Find("resource:metal")

    if repairedEngine == nil then
        repairedEngine = MeshToggleSystem.GetByName("wheel")
    end

    if repairedEngine == nil then
        print("BoatEngine: Engine meshes have not been registered!")
        tracy.ZoneEnd()
        return
    end
    if pb:CountItem(InventoryType.Player, repairedPartId) >= 1 then

        print("Repaired")
        if((pb:CountItem(InventoryType.Player, woodId) >= 10) and (pb:CountItem(InventoryType.Player, metalId) >= 10)) then
            pb:RemoveItem(InventoryType.Player, repairedPartId, 1)
            pb:RemoveItem(InventoryType.Player, woodId, 10)
            pb:RemoveItem(InventoryType.Player, metalId, 10)
            MeshToggleSystem.SetActive(repairedEngine, true)
        end
    else
        print("Can't do anything with the engine")
    end

    tracy.ZoneEnd()
end

function RenderUI()
end