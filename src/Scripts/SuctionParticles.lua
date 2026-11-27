SuctionParticles = SuctionParticles or {}

local particleEmitter = nil
local suction = false

function SuctionParticles.Initialize(node, range)
    
    local trans = node:GetTransform()
    local currentPosition = trans:GetPosition(TransformSpace.World)
    local forward = trans:GetForward(TransformSpace.World)

    local newPosition = Vec3.new(
        currentPosition.x + forward.x,
        currentPosition.y + forward.y,
        currentPosition.z + forward.z * range
    )

    trans:SetPosition(newPosition, TransformSpace.World)
    particleEmitter = node:GetParticleEmitterComponent()

    if not particleEmitter then
        print("SuctionPartcles: ParticleEmitterComponent not found")
        return
    end

    particleEmitter:Stop(true)
    suction = false
end

function SuctionParticles.StartSuction()
    if suction == true then
        return
    end

    if not particleEmitter then
        print("SuctionPartcles: ParticleEmitterComponent not found")
        return
    end

    particleEmitter:Play()
    suction = true;
end
 
function SuctionParticles.StopSuction()
    if not particleEmitter then
        print("SuctionPartcles: ParticleEmitterComponent not found")
        return
    end

    particleEmitter:Stop(true)
    suction = false;
end

return SuctionParticles