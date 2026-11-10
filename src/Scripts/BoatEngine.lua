
local interactableSystem = require("InteractableSystem")

local brokenPartId
local repairedPartId

function OnStart()
    local player = self:GetPlayerComponent()
    -- Initialize your item IDs here.

    interactableSystem.Register(self, function(playerNode)
        Interact(playerNode)
    end)
end

function Interact(playerNode)
    local playerComponent = playerNode:GetPlayerComponent()
    local pb = playerComponent:GetPlayerBehaviour()

    local brokenCount = pb:CountItem(
        InventoryType.Player, brokenPartId
    )

    local repairedCount = pb:CountItem(
        InventoryType.Player, repairedPartId
    )

    if brokenCount == 1 and repairedCount == 0 then
        print("Cannot repair the engine with a broken part")
    elseif repairedCount == 1 then
        print("Repaired")
        -- Perform repair.
    end
end

function OnDestroy()
    interactableSystem.Unregister(self)
end

function RenderUI()
end
