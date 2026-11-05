local counter = nil
-- Called when script starts
function OnStart()
    counter = self:GetNodeRemovalCount()
end

function OnUpdate()
    print(counter)
end

-- Called to render variables in the script
function RenderUI()
end
