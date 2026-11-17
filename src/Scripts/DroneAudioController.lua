local underwaterAmbianceSound = nil

local playing = false

-- Called when script starts
function OnStart()
    underwaterAmbianceSound = self:GetAudioPlayer("Underwater Ambiance")
    if underwaterAmbianceSound == nil then  
        print("Unable to find underwater ambiance sound")
        return
    end

    playing = false;
end

-- Called every frame
function OnUpdate(dt)
    if playing == false then
        print("Playing sound")
        underwaterAmbianceSound:Play()
        playing = true;
    end
end

function RenderUI()
end
