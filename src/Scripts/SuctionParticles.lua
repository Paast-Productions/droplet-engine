SuctionParticles = SuctionParticles or {}

local particleEmitter = nil

function SuctionParticles.Initialize(node)
    print("Initialize particles for suc")
    particleEmitter = node:GetParticleEmitterComponent()

    if not particleEmitter then
        print("SuctionPartcles: ParticleEmitterComponent not found")
        return
    end

    particleEmitter:Stop(true)
end

function SuctionParticles.StartSuction()
    if not particleEmitter then
        print("SuctionPartcles: ParticleEmitterComponent not found")
        return
    end
    particleEmitter:Play()
end
 
function SuctionParticles.StopSuction()
    if not particleEmitter then
        print("SuctionPartcles: ParticleEmitterComponent not found")
        return
    end

    particleEmitter:Stop(true)
end

return SuctionParticles