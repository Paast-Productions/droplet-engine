
-- Called when script starts
function OnStart()
    print("UITest")
end

-- Called every frame
function OnUpdate()
    local transform = self:GetTransform()
    local ui = self:GetUIComponent()
    if ui == nil then
        print("UIComponent nil")
        return
    end

    local pos = Vec2.new(transform:GetPosition().x, transform:GetPosition().y)
    local size = Vec2.new(transform:GetScale().x, transform:GetScale().y)

    print(pos.x, pos.y)
    print(size.x, size.y)


    if ui:IsPointInsideRectangle(pos, size) then
        print("works")
    end
end

-- Called to render variables in the script
function RenderUI()

end
