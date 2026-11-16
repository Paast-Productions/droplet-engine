
local smokeParticles = nil
local speed = 1.0

function GetSmokeParticles()
    smokeParticles = {}
    local smokeParticleNodes = self:GetChildren()
    
    -- Loop through children and fetch their transforms to smokeParticles
    for index, value in ipairs(smokeParticleNodes) do
        smokeParticles[index] = value:GetTransform()
    end
end

function OnStart()
    GetSmokeParticles()
end

function OnUpdate(dt)
    if smokeParticles == nil then
        GetSmokeParticles()
        if smokeParticles == nil then
            return  
        end
    end
    

end

function RenderUI()

	_, speed = ImGui.DragFloat("Speed", speed, 0.1, 0.0)
	
end