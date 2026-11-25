
local InteractableSystem = require("InteractableSystem")
local HatchSound = nil
local WantToPlayHatchSound = false

function OnStart()
    InteractableSystem.RegisterInteractable(self)
    GetSound()
    WantToPlayHatchSound = false
end

function OnUpdate(dt)
    if WantToPlayHatchSound then
        GetSound()
        HatchSound:Play()
        WantToPlayHatchSound = false
    end
end

function OnDestroy()
    InteractableSystem.UnregisterInteractable(self)
end

function OnTrigger(dt, node)
    tracy.ZoneBeginN("RepairBench OnTrigger")

    local player = node:GetPlayerComponent()

    if player == nil then
        tracy.ZoneEnd()
        return
    end

    local pb = player:GetPlayerBehaviour()

    if pb == nil then
        tracy.ZoneEnd()
        return
    end

    local ladder = self:GetLadderComponent()

    if ladder == nil then
        print("ajsing")
        tracy.ZoneEnd()
        return
    end

    ladder:BeginTransition()
    print("Going upp the ladder!! YIPPIIII!!!!")

    WantToPlayHatchSound = true

    tracy.ZoneEnd()
end

function GetSound()
    HatchSound = self:GetAudioPlayer("hatch-interact")
    if HatchSound == nil then  
        print("Unable to find hatch sound (ladder.lua)")
        return
    end
end

function RenderUI()
end
