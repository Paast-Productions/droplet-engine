SuctionParticles = SuctionParticles or {}

local particleEmitter = nil

function SuctionParticles.Start(thing)
    print(thing)
    particleEmitter = thing

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
    print("Partiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii")
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