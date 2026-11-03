local touchedTheMesh = false
local inventoryTable = {}


function Collector()
    table.insert(inventoryTable, playerInventory)
    for x = 1, #inventoryTable do
        print("Inventory Table: " .. tostring(inventoryTable[x]))
    end
end

function OnStart()
	print("Drone Upgrade Station")
    playerInventory = self:GetPlayerData()
end

function OnTrigger()
    touchedTheMesh = true
end

function OnUpdate()
    if touchedTheMesh then
        if Input:KeyPressed(Key.E) then
            Collector()
            touchedTheMesh = false
        end
    end
end

function RenderUI()
end
