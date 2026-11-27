SuctionParticles = SuctionParticles or {}

local particleEmitter = nil
local suction = false

function SuctionParticles.Initialize(node)
    print("Initialize particles for suc")
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
    print("papapapapapa")
    particleEmitter:Play()
    suction = true;
    print("poop")
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