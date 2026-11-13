
local fishSystem = require("FishSystem")
local scene

local interactionRange = 30.0
local interactionAngle = 30.0

local suctionSpeed = 4.0
local suctionDistance = 0.3
local maxSuctionTime = 2.0

local currentTarget = nil
local suctionElapsed = 0.0

local playerBehaviour
local woodId

function InteractWithFish(target)
    local playerPosition = self:GetTransform():GetPosition()
    local targetPosition = target.node:GetTransform():GetPosition()

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

function SuctionStart(target)
    currentTarget = target
    suctionElapsed = 0.0
end

function SuctionUpdate(dt)
    if currentTarget == nil then
        return
    end

    if not Input:KeyHeld(Key.E) then
        SuctionCancel()
        return
    end

    suctionElapsed = suctionElapsed + dt

    local playerPosition = self:GetTransform():GetPosition()
    local fishTransform = currentTarget.node:GetTransform()
    local fishPosition = fishTransform:GetPosition()

    local direction = Vec3.new( 
        playerPosition.x - fishPosition.x,
        playerPosition.y - fishPosition.y,
        playerPosition.z - fishPosition.z
    )

    local distance = math.sqrt(
        direction.x * direction.x +
        direction.y * direction.y +
        direction.z * direction.z
    )

    if distance <= suctionDistance or suctionElapsed >= maxSuctionTime then
        SuctionEnd()
        return
    end

    direction = Vec3.new(
        direction.x / distance,
        direction.y / distance,
        direction.z / distance
    )

    local movement = math.min(suctionSpeed * dt, distance)

    local newPosition = Vec3.new(
        fishPosition.x + direction.x * movement,
        fishPosition.y + direction.y * movement,
        fishPosition.z + direction.z * movement
    )

    fishTransform:SetPosition(newPosition)

end

function SuctionEnd()
    local fish = currentTarget

    currentTarget = nil
    suctionElapsed = 0.0

    fishSystem.UnregisterFish(fish.node, fish.type)
    scene:RemoveNode(fish.node)
    
    playerBehaviour:AddItem(InventoryType.Player, woodId, 10)
    print("Fish is yum: ")
end

function SuctionCancel()
    currentTarget = nil
    suctionElapsed = 0.0
end

function OnStart()
    scene = self:GetScene()

    local playerComponent = self:GetPlayerComponent()
    if not playerComponent then
        print("GetPlayerComponent returned nil")
        return
    end

    playerBehaviour = playerComponent:GetPlayerBehaviour()
    if not playerBehaviour then
        print("GetPlayerBehaviour returned nil")
        return
    end

    local registry = playerBehaviour:GetItemRegistry()
    if not registry then
        print("GetItemRegistry returned nil")
        return
    end

    woodId = registry:Find("resource:wood")

    if not woodId or not woodId:IsValid() then
        print("Wood item is not registered!")
        woodId = nil
    end
end

function OnUpdate(dt)

    if currentTarget ~= nil then
        SuctionUpdate(dt)
        return
    end

    for _, fish in ipairs(fishSystem.GetFishes()) do
        if Input:KeyPressed(Key.E) then
            if InteractWithFish(fish) then
                SuctionStart(fish)
                return
           end
        end
    end
    for _, upgrade in ipairs(fishSystem.GetUpgrads()) do        
        if Input:KeyPressed(Key.E) then
            if InteractWithFish(upgrade) then
                SuctionStart(upgrade)
                return
           end
        end
    end
end

function RenderUI()
    _, interactionRange = ImGui.SliderFloat("Interaction Range", interactionRange, 0.5, 100)
    _, interactionAngle = ImGui.SliderFloat("interactionAngle", interactionAngle, 0.5, 180)
    _, suctionSpeed = ImGui.SliderFloat("Suction Speed", suctionSpeed, 0.5, 20.0)
    _, suctionDistance = ImGui.SliderFloat("Suction Distance", suctionDistance, 0.5, 20.0)
    _, maxSuctionTime = ImGui.SliderFloat("Suction Time (max)", maxSuctionTime, 0.5, 20.0)
end