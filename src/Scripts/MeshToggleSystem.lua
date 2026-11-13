local MeshToggleSystem = {}

MeshToggleSystem.meshes = {}

function MeshToggleSystem.GetByName(name)
    for node, mesh in pairs(MeshToggleSystem.meshes) do
        if node:GetName() == name then
            return node
        end
    end

    return nil
end

function MeshToggleSystem.Register(node, meshComponent)
    if node == nil or meshComponent == nil then
        return
    end

    if MeshToggleSystem.meshes[node] ~= nil then
        return
    end

    MeshToggleSystem.meshes[node] = meshComponent
end

function MeshToggleSystem.Unregister(node)
    MeshToggleSystem.meshes[node] = nil
end

function MeshToggleSystem.SetActive(node, active)
    local meshComponent = MeshToggleSystem.meshes[node]

    if meshComponent == nil then
        return
    end

    meshComponent:SetActiveSelf(active)
end

function MeshToggleSystem.Toggle(node)
    local meshComponent = MeshToggleSystem.meshes[node]

    if meshComponent == nil then
        return
    end

    meshComponent:SetActiveSelf(not meshComponent:IsActive())
end

return MeshToggleSystem