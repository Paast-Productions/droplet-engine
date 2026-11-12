
local InteractableSystem = require("InteractableSystem")

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

    local brokenPartId = registry:Find("upgrade:broken_engine_part")
    local repairedPartId = registry:Find("upgrade:repaired_engine_part")

    if not brokenPartId:IsValid() or not repairedPartId:IsValid() then
        print("Engine part item IDs are invalid!")
        tracy.ZoneEnd()
        return
    end

    if pb:CountItem(InventoryType.Player, brokenPartId) == 1
        and pb:CountItem(InventoryType.Player, repairedPartId) == 0 then

        print("Cannot repair the engine with a broken part")

    elseif pb:CountItem(InventoryType.Player, repairedPartId) == 1 then

        print("Repaired")
        pb:RemoveItem(InventoryType.Player, repairedPartId, 1)

    else
        print("Can't do anything with the engine")
    end

    tracy.ZoneEnd()
end

function RenderUI()
end
