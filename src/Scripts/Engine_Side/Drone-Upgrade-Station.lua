
--make the math easier
local unique = nil
local itemRegistry = nil
local player = nil

-- variabels
local touchedTheMesh = false
local upgrades = {
    { id1 = "resource:metal",   label = "Heat armor",           mat1 = "metal",  pri1 = 5 },
    { id1 = "resource:wood",    label = "Increased inventory",  mat1 = "wood",   pri1 = 5 },
    { id1 = "resource:eel",     label = "Movement speed",       mat1 = "eel",    pri1 = 3 },
    { id1 = "resource:copper",  label = "Movement range",       mat1 = "copper", pri1 = 5 },
    { id1 = "resource:tin",     label = "Freeze armor",         mat1 = "tin",    pri1 = 7 },
    { id1 = "resource:bronze",  label = "Pressure",             mat1 = "bronze", pri1 = 5 }
}

local selected = nil

function OnStart()
    print("Drone Upgrade Station")
end

function Upgrades()
    if selected.label == "Heat armor" then
        local current = player:GetHeatResistance()
        player:SetHeatResistance(current + 10.0)
        print("Heat armor applied!")

    elseif selected.label == "Increased inventory" then
        local current = player:GetDroneInventorySize()
        player:SetDroneInventorySize(current + 1)
        print("Inventory increased!")

    elseif selected.label == "Movement speed" then
        local current = player:GetDroneMaxSpeed()
        player:SetDroneMaxSpeed(current + 100.0)
        print("Movement speed increased!")

    elseif selected.label == "Movement range" then
        local current = player:GetDroneMaxRange()
        player:SetDroneMaxRange(current + 1.0)
        print("Movement range increased!")

    elseif selected.label == "Freeze armor" then
        local current = player:GetFreezeResistance()
        player:SetFreezeResistance(current + 10.0)
        print("Freeze armor applied!")

    elseif selected.label == "Pressure" then
        local current = player:GetPressureResistance()
        player:SetPressureResistance(current + 10.0)
        print("Pressure tolerance increased!")
    end
end

function Crafting()
    print("Crafting station:")
    local amount

    local itemId1 = itemRegistry:Find(selected.id1)
    amount = player:CountItem(InventoryType.Player, itemId1)

    if (amount >= selected.pri1)  then
        amount = amount - selected.pri1
        print(amount .. " Rest for the first material")
        Upgrades()
        local succes = player:RemoveItem(InventoryType.Player, itemId1, selected.pri1)
    else
        print("Not enough resources to craft " .. selected.label)
    end
end

function RenderUI()
    for _, upgrade in ipairs(upgrades) do
        if ImGui.Button(upgrade.label) then
            if touchedTheMesh then
                selected = upgrade;
                print("It needs " .. "material: " .. selected.mat1 .. " price " .. selected.pri1)
                Crafting()
                touchedTheMesh = false
            end
        end
    end
end

function OnTrigger(dt, node)
    touchedTheMesh = true
    local playerComponent = node:GetPlayerComponent()

    player = playerComponent:GetPlayerBehaviour()
    unique = player:GetUniqueID()
    itemRegistry = player:GetItemRegistry() 
end
