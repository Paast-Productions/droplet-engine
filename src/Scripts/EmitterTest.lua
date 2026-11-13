local audioEmitter

-- Called when script starts
function OnStart()
    
end

-- Called every frame
function OnUpdate(dt)
    local audioEmitter = self:GetAudioEmitter("No need to be upset")
    if audioEmitter == nil then  
        print("Unable to find emitter")
        return
    end

    if Input:KeyPressed(Key.P) then
        audioEmitter:Play()
        print("P was pressed - Playing emitter")
    end
end

-- Called to render variables in the script
function RenderUI()

end
