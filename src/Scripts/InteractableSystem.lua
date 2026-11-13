local InteractableSystem = {}
InteractableSystem.interactables = {}

function InteractableSystem.RegisterInteractable(node)
    if node == nil then
        return
    end

    -- Avoid registering the same node twice.
    for _, interactable in ipairs(InteractableSystem.interactables) do
        if interactable == node then
            return
        end
    end

    print("Interacable node:", node)
    table.insert(InteractableSystem.interactables, node)
end

function InteractableSystem.UnregisterInteractable(node)
    if node == nil then
        return
    end

    for i, interactable in ipairs(InteractableSystem.interactables) do
        if interactable == node then
            table.remove(InteractableSystem.interactables, i)
            return
        end
    end

    print("Interactable was not registered")
end

function InteractableSystem.GetInteractables()
    return InteractableSystem.interactables
end

return InteractableSystem