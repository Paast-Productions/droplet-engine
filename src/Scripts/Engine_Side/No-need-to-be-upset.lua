local audioEmitter = nil
local transform = nil
local playing = true

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
end

-- Called every frame
function OnUpdate(dt)
    if not playing then
        audioEmitter:Play()
    end

	local toRotate = GLM.AngleAxis(Vec3.new(0.0, 1.0, 0.0), turnSpeed * dt)
	transform:Rotate(toRotate, TransformSpace.World)
end

function RenderUI()

end