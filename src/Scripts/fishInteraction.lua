
local fishSystem = require("FishSystem")
local scene

local interactioRange = 5.0
local interactionAngle = 30.0

local suctuonSpeed = 4.0
local suctionDistance = 0.3

local currentTarget = nil
local isSucking = false

function InteractWithFish(target)
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

    if distanceSquared > interactioRange * interactioRange then
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


function OnStart()
    scene = SceneManagerInstance:GetScene("TutorialScecne")

    if not scene then
        print("Scene not found")
        return
    end

end

function OnUpdate(dt)
    for _, fish in ipairs(fishSystem:GetFishes()) do
        if InteractWithFish(fish) then
           if Input:KeyHeld(Key.E) then
                scene:RemoveNode(fish)
                print("Succc")
           end
        end
    end
end

function RenderUI()
end