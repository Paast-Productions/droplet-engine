local pb = nil

local underwaterAmbianceSound = nil
local playingAmbiance = false

local freezeSFX = nil
local playingFreeze = false

-- Called when script starts
function OnStart()
    local player = self:GetPlayerComponent()
    if player ~= nil then
        pb = player:GetPlayerBehaviour()
    end

    underwaterAmbianceSound = self:GetAudioPlayer("Underwater Ambiance")
    if underwaterAmbianceSound == nil then  
        print("Unable to find underwater ambiance sound")
        return
    end

    freezeSFX = self:GetAudioPlayer("Freeze")
    if freezeSFX == nil then  
        print("Unable to find freeze sound")
        return
    end

    playingAmbiance = false
    playingFreeze = false
end

function OnUpdate(dt)
    -- Ambiance Sound
    if playingAmbiance == false then
        print("Playing sound")
        underwaterAmbianceSound:Play()
        playingAmbiance = true
    end

    -- Freeze Sound
    currentFreeze = pb:GetFreezeLevel()

    -- Over 200 -> Play freeze
    if currentFreeze >= 200 and playingFreeze == false then
        freezeSFX:Play()
        playingFreeze = true

    -- Under 200 -> Don't play freeze
    elseif currentFreeze < 200 and playingFreeze == true then
        freezeSFX:Stop()
        playingFreeze = false
    end
end

function RenderUI()
end
