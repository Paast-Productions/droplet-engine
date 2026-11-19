
local InteractableSystem = require("InteractableSystem")
local transform = nil
local interacted = false
local playerTransform = nil
local physics = nil

function OnStart()
    InteractableSystem.RegisterInteractable(self)
    transform = self:GetTransform()
end

function OnDestroy()
    InteractableSystem.UnregisterInteractable(self)
end

function OnTrigger(dt, node)

    local player = node:GetPlayerComponent()

    if player == nil then
        return
    end

    local pb = player:GetPlayerBehaviour()

    if pb == nil then
        return
    end
    playerTransform = node:GetTransform()
    interacted = not interacted
end

function OnUpdate(dt)

    if interacted == true then
        transform:SetPosition(GLM.V3Add(playerTransform:GetPosition(), playerTransform:GetForward()), TransformSpace.World)
    end
end

function RenderUI()
end
    