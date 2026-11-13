
local MeshToggleSystem = require("MeshToggleSystem")

function OnStart()
    local mesh = self:GetMeshComponent()

    if mesh == nil then
        print("MeshToggle: No MeshComponent found on", self:GetName())
        return
    end

    MeshToggleSystem.Register(self, mesh)
end

function OnDestroy()
    MeshToggleSystem.Unregister(self)
end
