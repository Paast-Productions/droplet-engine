local pb = nil
local health = nil
local parent = nil
local originalScale = nil
-- Called when script starts
function OnStart()
    parent = self:GetParent()
    trans = self:GetTransform()
    --originalScale = trans:GetScale()
    --print("OriginalScale:", originalScale.x, " ", originalScale.y, " ", originalScale.z)
end

-- Called every frame
function OnUpdate(dt)

    if parent == nil then
        return
    end

    if health == nil then
        health = parent:GetHealthComponent()

        if health ~= nil then
            print("SUCCESS: HealthComponent found!")
        end

        return
    end

    if originalScale == nil then
        originalScale = trans:GetScale()
        print("Found originalScale")
        print("OriginalScale:", originalScale.x, " ", originalScale.y, " ", originalScale.z)

    end

    local currentHealth = health:GetHealth()
    local maxHealth = health:GetMaxHealth()

    if maxHealth <= 0 then 
        return
    end

    local healthPercentage = currentHealth / maxHealth
    --print("Health precentage: ", healthPercentage)

    trans:SetScale(Vec3.new(originalScale.x, originalScale.y, originalScale.z * healthPercentage))

end

-- Called to render variables in the script
function RenderUI()

end
