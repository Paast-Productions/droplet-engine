local audioEmitter = nil
local audioBehaviour = nil;
local transform = nil
local playing = false

local turnSpeed = 1.0

-- Called when script starts
function OnStart()
    print("Script Start")
    audioEmitter = self:GetAudioEmitter("No need to be upset")
    if audioEmitter == nil then  
        print("Unable to find emitter")
        return
    end

    transform = self:GetTransform()
    if transform == nil then  
        print("Unable to find transform")
        return
    end

    audioBehaviour = self:GetAudioBehaviour()
    if audioBehaviour == nil then  
        print("Unable to find audio behaviour")
        return
    end

    playing = false;
end

-- Called every frame
function OnUpdate(dt)
    if playing == false then
        audioEmitter:Play()
        audioBehaviour:PlayTrack("The big blue intro", 5.0)
        playing = true;
    end

	local toRotate = GLM.AngleAxis(Vec3.new(0.0, 1.0, 0.0), turnSpeed * dt)
	transform:Rotate(toRotate, TransformSpace.World)
end

function RenderUI()

end     