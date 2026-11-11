local interactableSystem = require("InteractableSystem")

local interactionRange = 30.0
local interactionAngle = 30.0

local function CanInteract(target)
    local playerPosition = self:GetTransform():GetPosition()
    local targetPosition = target:GetTransform():GetPosition()

    local forward = self:GetTransform():GetForward()

    local direction = Vec3.new(
        targetPosition.x - playerPosition.x,
        targetPosition.y - playerPosition.y,
        targetPosition.z - playerPosition.z
    )

    local distanceSquared =
        direction.x * direction.x +
        direction.y * direction.y +
        direction.z * direction.z

    if distanceSquared > interactionRange * interactionRange then
        return false
    end

    if distanceSquared < 0.0001 then
        return true
    end

    local distance = math.sqrt(distanceSquared)

    direction = Vec3.new(
        direction.x / distance,
        direction.y / distance,
        direction.z / distance
    )

    local dot =
        forward.x * direction.x +
        forward.y * direction.y +
        forward.z * direction.z

    local threshold = math.cos(math.rad(interactionAngle))

    return dot >= threshold
end

local function TryInteract(target)
    local interactable = target:GetInteractableComponent()

    if interactable == nil then
        print("naj")
        return false
    end

    interactable:Interact(self)
    print("yes")
    return true
end

function OnUpdate(dt)
    if not Input:KeyPressed(Key.E) then
        return
    end

    for _, target in ipairs(interactableSystem.GetInteractables()) do
        if target ~= nil and CanInteract(target) then
            print("can interact")
            if TryInteract(target) then
                return
            end
        end
        print("yo")
    end
end

function RenderUI()
    _, interactionRange = ImGui.SliderFloat(
        "Interaction Range", interactionRange, 0.5, 100.0
    )

    _, interactionAngle = ImGui.SliderFloat(
        "Interaction Angle", interactionAngle, 0.5, 180.0
    )
end