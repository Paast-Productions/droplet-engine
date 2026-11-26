
local suctionSystem = require("SuctionSystem")
local suctionParticles = require("SuctionParticles")
local scene

local interactionRange = 5.0
local interactionAngle = 30.0
local suctionSpeed = 4.0
local suctionDistance = 0.3

local activeTargets = {}

local playerBehaviour
local audioplayerMusic
local audioplayerCollect
local audioplayerSucction
local playingSucction = false

function FindNewTargets()

    for _, fish in ipairs(suctionSystem.GetFishes()) do
        TryAddtarget(fish)
    end

    for _, upgrade in ipairs(suctionSystem.GetUpgrades()) do
        TryAddtarget(upgrade)
    end

    for _, part in ipairs(suctionSystem.GetBrokenParts()) do
        TryAddtarget(part)
    end
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

function IsAlreadySucking(target)

    for _, active in ipairs(activeTargets) do
        if active.node == target.node then
            return true
        end
    end

    return false
end

function TryAddtarget(target)

    if #activeTargets >= 3 then
        return
    end

    if IsAlreadySucking(target) then
        return
    end

    if InteractWith(target) then
        SuctionStart(target)
    end
end

function SuctionStart(target) 
    local suctionTarget = {
        collectible = target,
        node = target.node,
        type = target.type
    }

    table.insert(activeTargets, suctionTarget)
    print("Started suction:", target.node:GetName())
end

function SuctionUpdateTarget(target, dt)

    local playerPosition = self:GetTransform():GetPosition(TransformSpace.World)
    local targetTransform = target.node:GetTransform(TransformSpace.World)
    local targetPosition = targetTransform:GetPosition(TransformSpace.World)

    local direction = Vec3.new(
        playerPosition.x - targetPosition.x,
        playerPosition.y - targetPosition.y,
        playerPosition.z - targetPosition.z
    )

    local distance = math.sqrt(
        direction.x * direction.x +
        direction.y * direction.y +
        direction.z * direction.z
    )

    if distance <= suctionDistance then
        return true
    end
        if distance < 0.0001 then
        return true
    end

    direction = Vec3.new(
        direction.x / distance,
        direction.y / distance,
        direction.z / distance
    )

    local movement = math.min(suctionSpeed * dt, distance)

    local newPosition = Vec3.new(
        targetPosition.x + direction.x * movement,
        targetPosition.y + direction.y * movement,
        targetPosition.z + direction.z * movement
    )

    targetTransform:SetPosition(newPosition, TransformSpace.World)

    return false
end

function SuctionUpdateAll(dt)

    for i = #activeTargets, 1, -1 do
        
        local target = activeTargets[i]

        if SuctionUpdateTarget(target, dt) then

            SuctionEnd(target)
            table.remove(activeTargets, i)
        end
    end
end

function SuctionEnd(target)

    local collectible = target.collectible
    local registry = playerBehaviour:GetItemRegistry()
    
    if collectible.item and collectible.item:IsValid() then
        playerBehaviour:AddItem(
            InventoryType.Player,
            collectible.item,
            collectible.amount
        )
        audioplayerCollect:Play()
        print("Collected item:", collectible.amount)
    else
        print("Collectible has invalid ItemId", collectible.item)
    end

    local repairedPartId = registry:Find("upgrade:repaired_engine_part")

    if collectible.item and collectible.item:IsValid() then
        if collectible.item == repairedPartId then
            audioplayerMusic:Play()
        end
        --HACK -- This is for the game test, picking up the engine starts music
    end

    suctionSystem.UnregisterCollectible(collectible.node)
    scene:RemoveNode(collectible.node)
end

function SuctionCancel()
    suctionParticles.StopSuction()

    if #activeTargets == 0 then
        return
    end
    
    activeTargets = {}
    print("We stop all succ")
end

function UpdateSuctionSound()
    
   if Input:KeyHeld(Key.E) then
        print("Mouse succ")
        if not playingSucction then
            audioplayerSucction:Play()
            playingSucction = true
            print("start suction sound")
        end

    else
        if playingSucction then
            audioplayerSucction:Stop()
            playingSucction = false
            print("Stopping suction sound")
        end
    end
end

function OnStart()
    scene = self:GetScene()
    audioplayerMusic = self:GetAudioPlayer("The Big Blue(Vorbis 96kHz Mono)") --HACK
    if audioplayerMusic == nil then
        print("Could not find audio player for The Big Blue")
    end

    audioplayerCollect = self:GetAudioPlayer("Fwomp")
    if audioplayerCollect == nil then
        print("Could not find audioplayer for suction-two")
    end

    audioplayerSucction = self:GetAudioPlayer("leaf-blower")
    if audioplayerSucction == nil then
        print("Could not find audioplayer for 'leaf-blower'")
    end

    playingSucction = false

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

    local particleEmitter = self:GetParticleEmitterComponent()
    suctionParticles.Start(particleEmitter)

end

function OnUpdate(dt)
    UpdateSuctionSound()

    if not Input:KeyHeld(Key.E) then
        SuctionCancel()
        return
    else
        suctionParticles.StartSuction()
        FindNewTargets()
        SuctionUpdateAll(dt)
    end

end

function RenderUI()
    _, interactionRange = ImGui.SliderFloat("Interaction Range", interactionRange, 0.5, 50)
    _, interactionAngle = ImGui.SliderFloat("interactionAngle", interactionAngle, 0.5, 180)
    _, suctionSpeed = ImGui.SliderFloat("Suction Speed", suctionSpeed, 0.5, 20.0)
    _, suctionDistance = ImGui.SliderFloat("Suction Distance", suctionDistance, 0.5, 20.0)
end