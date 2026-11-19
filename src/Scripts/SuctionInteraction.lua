
local suctionSystem = require("SuctionSystem")
local scene

local interactionRange = 5.0
local interactionAngle = 30.0

local suctionSpeed = 4.0
local suctionDistance = 0.3

local currentTarget = nil
local suctionElapsed = 0.0

local playerBehaviour
local audioplayer


function FindTarget()
     for _, fish in ipairs(suctionSystem.GetFishes()) do
        if InteractWith(fish) then
            return fish
        end
    end
    for _, upgrade in ipairs(suctionSystem.GetUpgrades()) do        
        if InteractWith(upgrade) then
            return upgrade
        end
    end
    for _, brokenPart in ipairs(suctionSystem.GetBrokenParts()) do        
        if InteractWith(brokenPart) then
            return brokenPart
        end
    end

    return nil
end

function InteractWith(target)
    local playerPosition = self:GetTransform():GetPosition(TransformSpace.World)
    local targetPosition = target.node:GetTransform():GetPosition(TransformSpace.World)


    local forward = self:GetTransform():GetForward(TransformSpace.World)

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
    print("suctionStart")
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

    local playerTransform = self:GetTransform()
    local fishTransform = currentTarget.node:GetTransform()

    local playerPosition = playerTransform:GetPosition(TransformSpace.World)

    local fishPosition = fishTransform:GetPosition(TransformSpace.World)

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

    if distance <= suctionDistance then
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

    fishTransform:SetPosition(newPosition, TransformSpace.World)
end

function SuctionEnd()

    if currentTarget == nil then
        return
    end

    local registry = playerBehaviour:GetItemRegistry()
    local collectible = currentTarget
    
    if collectible.item and collectible.item:IsValid() then
        playerBehaviour:AddItem(
            InventoryType.Player,
            collectible.item,
            collectible.amount
        )
        print("Collected item:", collectible.amount)
    else
        print("Collectible has invalid ItemId", collectible.item)
    end
    local repairedPartId = registry:Find("upgrade:repaired_engine_part")

    if collectible.item and collectible.item:IsValid() then
        if collectible.item == repairedPartId then
            audioplayer:Play()
        end
        --HACK -- This is for the game test, picking up the engine starts music
    end

    currentTarget = nil
    suctionElapsed = 0.0

    suctionSystem.UnregisterCollectible(collectible.node)
    scene:RemoveNode(collectible.node)
end

function SuctionCancel()
    currentTarget = nil
    suctionElapsed = 0.0
end

function OnStart()
    scene = self:GetScene()
    audioplayer = self:GetAudioPlayer("The Big Blue(Vorbis 96kHz Mono)") --HACK
    if not audioplayer then
        print("Could not find audio player")
    end
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

end

function OnUpdate(dt)

    if not Input:KeyHeld(Key.E) then
        if currentTarget ~= nil then
            SuctionCancel()
        end
        return
    end

    if currentTarget ~= nil then
        SuctionUpdate(dt)
        return
    end

    if currentTarget ~= nil then
        SuctionUpdate(dt)
        return
    end

    local target = FindTarget()

    if target ~= nill
     then
        SuctionStart(target)
        SuctionUpdate(dt)
     end

end

function RenderUI()
    _, interactionRange = ImGui.SliderFloat("Interaction Range", interactionRange, 0.5, 50)
    _, interactionAngle = ImGui.SliderFloat("interactionAngle", interactionAngle, 0.5, 180)
    _, suctionSpeed = ImGui.SliderFloat("Suction Speed", suctionSpeed, 0.5, 20.0)
    _, suctionDistance = ImGui.SliderFloat("Suction Distance", suctionDistance, 0.5, 20.0)
end