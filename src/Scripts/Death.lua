
-- Called when script starts
function OnStart()

end

-- Called every frame
function OnUpdate(dt)
    local health = self:GetHealthComponent()
    
    if (health:GetHealth() == 0) then
        local parent = self:GetParent()
        print("Parent", parent)
        local parentTrans = parent:GetTransform()

        print("ParentTrans: ", parentTrans)
        local maxHealth = health:GetMaxHealth()
        health:Heal(maxHealth)
        --self:SetTransform(Vec3.new(parentTrans.x, parentTrans.y, parentTrans.z)
        print("Is it this simple?")
    end
end

-- Called to render variables in the script
function RenderUI()

end
