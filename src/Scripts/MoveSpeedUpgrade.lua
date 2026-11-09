local movementUpgradeStage = 1
local movementSpeedModifier = 1.0

-- Called when script starts
function OnStart()

end

-- Called every frame
function OnUpdate(dt)

end

local function SetMovementUpgrade(stage)
    movementUpgradeStage = stage

    if movementUpgradeStage == 1 then
        movementSpeedModifier = 1.0
    elseif movementUpgradeStage == 2 then
        movementSpeedModifier = 1.5
    elseif movementUpgradeStage == 3 then
        movementSpeedModifier = 2.0
    end
    print("Set movement upgrade stage to " .. movementUpgradeStage .. " - speed modifier is now at " .. movementSpeedModifier)
end

function RenderUI()
    ImGui.Spacing()

    if ImGui.Button("Stage 1") and movementUpgradeStage ~= 1 then
        SetMovementUpgrade(1)
    elseif ImGui.Button("Stage 2") and movementUpgradeStage ~= 2 then
        SetMovementUpgrade(2)
    elseif ImGui.Button("Stage 3") and movementUpgradeStage ~= 3 then
        SetMovementUpgrade(3)
    end
end