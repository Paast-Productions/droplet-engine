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

    local brokenPartId = registry:Find("upgrade:broken_engine_part")
    local repairedPartId = registry:Find("upgrade:repaired_engine_part")

    if repairedEngine == nil then
        repairedEngine = MeshToggleSystem.GetByName("wheel")
    end

    if repairedEngine == nil then
        print("BoatEngine: Engine meshes have not been registered!")
        tracy.ZoneEnd()
        return
    end

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

        -- Switch engine meshes.
        --MeshToggleSystem.SetActive(brokenEngine, false)
        MeshToggleSystem.SetActive(repairedEngine, true)

    else
        print("Can't do anything with the engine")
    end

    tracy.ZoneEnd()
end

function RenderUI()
end