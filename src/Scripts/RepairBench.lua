
local InteractableSystem = require("InteractableSystem")

function OnStart()
    InteractableSystem.RegisterInteractable(self)
end

function OnDestroy()
    InteractableSystem.UnregisterInteractable(self)
end

function OnTrigger(dt, node)
    tracy.ZoneBeginN("RepairBench OnTrigger")

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
    local woodId = registry:Find("resource:wood")

    if not brokenPartId:IsValid()
        or not repairedPartId:IsValid()
        or not woodId:IsValid() then

        print("One or more required item IDs are invalid!")
        tracy.ZoneEnd()
        return
    end

    if pb:CountItem(InventoryType.Player, brokenPartId) >= 1 then

        if pb:CountItem(InventoryType.Player, woodId) >= 5 then

            print("Used 5 wood and repaired engine part")

            pb:RemoveItem(InventoryType.Player, woodId, 5)
            pb:RemoveItem(InventoryType.Player, brokenPartId, 1)
            pb:AddItem(InventoryType.Player, repairedPartId, 1)

        else
            print("Not enough wood")
        end

    else
        print("You don't have a broken engine part")
    end

    tracy.ZoneEnd()
end

function RenderUI()
end
