 

function OnStart()
    print("temp raycast")
end

function OnUpdate()
    if Input:KeyPressed(Key.E) then
        self:RemoveNode();
    end
end

function RenderUI()
end