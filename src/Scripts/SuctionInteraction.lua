
local suctionSystem = require("SuctionSystem")
local scene

local interactionRange = 30.0
local interactionAngle = 30.0

local suctionSpeed = 4.0
local suctionDistance = 0.3
local maxSuctionTime = 2.0

local currentTarget = nil
local suctionElapsed = 0.0

local playerBehaviour
local audioplayerMusic
local audioplayerCollect
local audioplayerSucction
local playingSucction = false

function InteractWith(target)
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

    print("Succc Succc", newPosition.x, newPosition.y, newPosition.z)
    fishTransform:SetPosition(newPosition)

end

function SuctionEnd()



    local registry = playerBehaviour:GetItemRegistry()
    local collectible = currentTarget
    
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

    currentTarget = nil
    suctionElapsed = 0.0

    suctionSystem.UnregisterCollectible(collectible.node)
    scene:RemoveNode(collectible.node)
end

function SuctionCancel()
    currentTarget = nil
    suctionElapsed = 0.0
end

function UpdateSuctionSound()
    if Input:KeyHeld(Key.E) then
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

end

function OnUpdate(dt)
    UpdateSuctionSound()

    if currentTarget ~= nil then
        SuctionUpdate(dt)
        return
    end

    for _, fish in ipairs(suctionSystem.GetFishes()) do
        if Input:KeyPressed(Key.E) then
            if InteractWith(fish) then 
                SuctionStart(fish)
                return
           end
        end
    end
    for _, upgrade in ipairs(suctionSystem.GetUpgrads()) do        
        if Input:KeyPressed(Key.E) then
            if InteractWith(upgrade) then
                SuctionStart(upgrade)
                return
           end
        end
    end
    for _, brokenPart in ipairs(suctionSystem.GetBrokenParts()) do        
        if Input:KeyPressed(Key.E) then
            if InteractWith(brokenPart) then
                SuctionStart(brokenPart)
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