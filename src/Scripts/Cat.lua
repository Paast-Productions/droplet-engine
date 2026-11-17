
local InteractableSystem = require("InteractableSystem")
local transform = nil
local interacted = false
local timer = 0

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

    interacted = true
end

function OnUpdate(dt)

    if interacted == true then
        timer = timer + dt
        transform:Rotate(GLM.AngleAxis(Vec3.new(0.0, 1.0, 0.0), dt * 10), TransformSpace.World)

        if timer > 4 then
            timer = timer + dt * 100
            transform:Move(Vec3.new(0.0, dt * timer / 10 + math.cos(timer / 6) * dt, 0.0), TransformSpace.World)
        else
            transform:Move(Vec3.new(0.0, dt * timer / 10 + math.cos(timer) * dt, 0.0), TransformSpace.World)
        end

        if transform:GetPosition().y > 100 then
            interacted = false
        end
    end
end

function RenderUI()
end
