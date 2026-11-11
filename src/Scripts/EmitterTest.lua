
-- Called when script starts
function OnStart()
    tracy.ZoneBeginN("PlayerController OnStart")

    print("New AudioEmitter")
	trans = self:GetTransform()
	
    tracy.ZoneEnd()
end

-- Called every frame
function OnUpdate(dt)
    local emitter = self:GetAudioEmitter("Gupta")
    if emitter == nil then
        print("not")
        return
    end
    emitter:PlaySound()
    print("Gupta emitter found")
end

-- Called to render variables in the script
function RenderUI()

end
