--local MeshToggleSystem = require("MeshToggleSystem")

local pb = nil
local trans = nil

-- Called when script starts
function OnStart()
    local player = self:GetPlayerComponent()
    if player ~= nil then
        pb = player:GetPlayerBehaviour()
    end
end

-- Called every frame
function OnUpdate(dt)
    if pb == nil then
        return
    end

    local freezeLevel = pb:GetFreezeLevel()
    if freezeLevel > 200 then
        -- Set mesh active
        print("FreezeLevel is: ", freezeLevel)
    end
end

-- Called to render variables in the script
function RenderUI()

end
