local SeaAmbianceSound = nil
local playingAmbiance = false

-- Called when script starts
function OnStart()
    SeaAmbianceSound = self:GetAudioPlayer("Sea-at-night")
    if SeaAmbianceSound == nil then  
        print("Unable to find ambiance sound (seaAmbiance.lua)")
        return
    end

    playingAmbiance = false
end

function OnUpdate(dt)
    -- Ambiance Sound
    if playingAmbiance == false then
        SeaAmbianceSound:Play()
        playingAmbiance = true
    end
end

function RenderUI()
end
