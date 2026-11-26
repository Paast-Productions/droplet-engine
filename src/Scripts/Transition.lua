-- We want this so it can trigger the "OnTrigger" function
local InteractableSystem = require("InteractableSystem")

function OnStart()
    InteractableSystem.RegisterInteractable(self)
end

function OnUpdate(dt)
end

function OnDestroy()
    InteractableSystem.UnregisterInteractable(self)
end

function OnTrigger(dt, node)
    tracy.ZoneBeginN("TransitionComponent OnTrigger")

    print("Trying transition! (testTransition.lua)")

    local player = node:GetPlayerComponent()

    if player == nil then
        tracy.ZoneEnd()
        print("No player component found (testTransition.lua)")
        return
    end

    local pb = player:GetPlayerBehaviour()

    if pb == nil then
        tracy.ZoneEnd()
        print("No player behaviour found (testTransition.lua)")
        return
    end

    local transition = self:GetTransitionComponent()

    if transition == nil then
        print("Transition failed")
        tracy.ZoneEnd()
        return
    end

    transition:BeginTransition()
    print("Transition complete!")

    tracy.ZoneEnd()
end

function RenderUI()
end
