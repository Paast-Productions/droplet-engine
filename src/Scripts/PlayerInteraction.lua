local interactableSystem = require("InteractableSystem")

local interactionRange = 30.0
local scene

local function TryInteract(target)
    if target == nil then
        return false
    end

    local interactable = target:GetInteractableComponent()

    if interactable == nil then
        return false
    end

    interactable:Interact(self)
    return true
end

function OnStart()
    scene = self:GetScene()
end

function OnUpdate(dt)
    if not Input:KeyPressed(Key.E) then
        return
    end

    local origin = self:GetTransform():GetPosition()
    local direction = self:GetTransform():GetForward()

    local ray = Ray.new(origin, direction)

    local target, hit = scene:RaycastNode(ray)

    if not hit.didHit or target == nil then
        return
    end

    if hit.distance > interactionRange then
        return
    end

    TryInteract(target)
end

function RenderUI()
    _, interactionRange = ImGui.SliderFloat(
        "Interaction Range", interactionRange, 0.5, 100.0
    )
end