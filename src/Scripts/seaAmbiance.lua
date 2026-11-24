local SeaAmbianceSound = nil

-- Called when script starts
function OnStart()
    SeaAmbianceSound = self:GetAudioPlayer("Sea-at-night")
    if SeaAmbianceSound == nil then  
        print("Unable to find ambiance sound (seaAmbiance.lua)")
        return
    end
end

function OnUpdate(dt)
    -- Ambiance Sound
    if not SeaAmbianceSound:IsPlaying() then
        SeaAmbianceSound:Play()
    end
end

function RenderUI()
end
