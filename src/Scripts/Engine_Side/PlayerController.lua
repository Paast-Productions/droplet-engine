local trans = nil

local mouseSensitivity = 0.0025   -- radians per pixel
local moveSpeed = 5.0

-- Variables for stepping sound
local AudioPlayer = nil
local StoppedMovingTime = 0
local WalkSoundStopDelay = 0.25

function OnStart()
    tracy.ZoneBeginN("PlayerController OnStart")

    print("New PlayerController")
	trans = self:GetTransform()

    AudioPlayer = self:GetAudioPlayer("metal-footsteps")
    if AudioPlayer == nil then  
        print("Unable to find AudioPlayer (PlayerController.lua)")
    end

    StoppedMovingTime = 0
	
    tracy.ZoneEnd()
end

function OnUpdate(dt)
    tracy.ZoneBeginN("PlayerController OnUpdate")

    local IsWalking = false
    StoppedMovingTime = StoppedMovingTime + dt

    if trans == nil then
	    trans = self:GetTransform()
    end
    
    local physComp = self:GetPhysicsComponent()
    if physComp == nil then
        return
    end
    
    -- HACK: This rotation code should've been in CameraController.lua but doesn't work there
    local dx = Input:GetDeltaMouseX()
    local yawDelta = -dx * mouseSensitivity
    trans:RotateAxis(yawDelta, Vec3.new(0.0, 1.0, 0.0), TransformSpace.World)

    local walkDir = Vec3.new(0.0, 0.0, 0.0)
	if Input:KeyHeld(Key.W) then
        walkDir = GLM.V3Add(walkDir, trans:GetForward())
    end

    if Input:KeyHeld(Key.A) then
        walkDir = GLM.V3Add(walkDir, trans:GetRight())
    end

    if Input:KeyHeld(Key.S) then
        walkDir = GLM.V3Sub(walkDir, trans:GetForward())
    end

    if Input:KeyHeld(Key.D) then
        walkDir = GLM.V3Sub(walkDir, trans:GetRight())
    end

    if walkDir.x ~= 0.0 or walkDir.y ~= 0.0 or walkDir.z ~= 0.0 then
        walkDir = GLM.V3MulF(GLM.V3Normalize(walkDir), moveSpeed)
    end
    walkDir.y = physComp:GetVelocity().y
    physComp:SetVelocity(walkDir)

    if GLM.V3Length(walkDir) > 1 then
        IsWalking = true
        StoppedMovingTime = 0
    end

    -- Handle walking sound
    if IsWalking == false then
        if StoppedMovingTime > WalkSoundStopDelay then
            if AudioPlayer:IsPlaying() then
                AudioPlayer:Stop()
            end
        end
    elseif IsWalking == true then
        if not AudioPlayer:IsPlaying() then
            AudioPlayer:Play()
        end
    end
    
    tracy.ZoneEnd()
end

function RenderUI()
    ImGui.Text("WARNING: This script rotates the node around the world y-axis using the mouse.")
end