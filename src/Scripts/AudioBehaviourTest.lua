local audioBehaviour = nil
local firstUpdate = false

-- Called when script starts
function OnStart()
    audioBehaviour = self:GetAudioBehaviour("Music")
    if audioBehaviour == nil then  
        print("Unable to find audio behaviour")
        return
    end

    firstUpdate = true
end

-- Called every frame
function OnUpdate(dt)
    if firstUpdate == true then
        audioBehaviour:PlayTrack("The big blue intro", 2.0)
        firstUpdate = false
    end
end

-- Called to render variables in the script
function RenderUI()

end
