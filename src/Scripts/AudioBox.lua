
local audioPlayerComp = nil
local timesTriggered = 0

-- Called when script starts
function OnStart()
    audioPlayerComp = self:GetAudioPlayer("No need to be upset")

    if audioPlayerComp == nil then
        print("Unable to find audio player")
    end



end

-- Called every frame
function OnUpdate(dt)

end

function OnTrigger(dt, node)
    if timesTriggered == 0 then
        audioPlayerComp:Play()
        timesTriggered = 1
    end

    
end

-- Called to render variables in the script
function RenderUI()

end
