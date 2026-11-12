
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

    print("Going upp the ladder!! YIPPIIII!!!!")

    tracy.ZoneEnd()
end

function RenderUI()
end
