--local MeshToggleSystem = require("MeshToggleSystem")

local pb = nil
local trans = nil
local level = nil

-- Called when script starts
function OnStart()
    parent = self:GetParent()
end

-- Called every frame
function OnUpdate(dt)
    local hazard = self:GetHazardComponent()

    level = hazard:GetDamage()
    if parent == nil then
        return
    end
    
    local playerComponent = parent:GetPlayerComponent()

    pb = playerComponent:GetPlayerBehaviour()

    if pb == nil then
        print("pb is niL!=!==!")
        return
    end

    local freezeLevel = pb:GetFreezeLevel() / 200
    local mesh = self:GetMeshComponent()
    
    if freezeLevel >= level and freezeLevel < level + 1 then
        mesh:SetActiveSelf(true)
    else
        mesh:SetActiveSelf(false)
    end
end

-- Called to render variables in the script
function RenderUI()

end
