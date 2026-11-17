local InteractableSystem = require("InteractableSystem")
local MeshToggleSystem = require("MeshToggleSystem")

local brokenEngine
local repairedEngine

local engineRepairSound = nil;
local engineRepairFailedSound = nil;

function OnStart()
    InteractableSystem.RegisterInteractable(self)

    engineRepairSound = self:GetAudioEmitter("Repair Engine")
    if engineRepairSound == nil then  
        print("Unable to find repair sound emitter")
        return
    end

    engineRepairFailedSound = self:GetAudioEmitter("Repair Engine Failed")
    if engineRepairSound == nil then  
        print("Unable to find repair failed sound emitter")
        return
    end

end

function OnDestroy()
    InteractableSystem.UnregisterInteractable(self)
end

function OnTrigger(dt, node)
    tracy.ZoneBeginN("BoatEngine OnTrigger")

    local player = node:GetPlayerComponent()

    if player == nil then
        tracy.ZoneEnd()
        return
    end

    local pb = player:GetPlayerBehaviour()

    if pb == nil then
        tracy.ZoneEnd()
        return
    end

    local registry = pb:GetItemRegistry()

    if registry == nil then
        print("Item registry is unavailable!")
        tracy.ZoneEnd()
        return
    end

    local repairedPartId = registry:Find("upgrade:repaired_engine_part")
    local woodId = registry:Find("resource:wood")
    local metalId = registry:Find("resource:metal")

    if repairedEngine == nil then
        repairedEngine = MeshToggleSystem.GetByName("wheel")
    end

    if repairedEngine == nil then
        print("BoatEngine: Engine meshes have not been registered!")
        tracy.ZoneEnd()
        return
    end
    if pb:CountItem(InventoryType.Player, repairedPartId) >= 1 then

        print("Repaired")
        engineRepairSound:Play()

        if((pb:CountItem(InventoryType.Player, woodId) >= 10) and (pb:CountItem(InventoryType.Player, metalId) >= 10)) then
            pb:RemoveItem(InventoryType.Player, repairedPartId, 1)
            pb:RemoveItem(InventoryType.Player, woodId, 10)
            pb:RemoveItem(InventoryType.Player, metalId, 10)
            local Children = self:GetChildren()
            for index, value in ipairs(Children) do
                print(value:GetName())
                if(value:GetName() == "BlackParticleSystem") then
                    value:SetActiveSelf(false)
                end
                if(value:GetName() == "WhiteParticleSystem") then
                    value:SetActiveSelf(true)
                end
            end
            MeshToggleSystem.SetActive(repairedEngine, true)
        end
    else
        print("Can't do anything with the engine")
        engineRepairFailedSound:Play()
    end

    tracy.ZoneEnd()
end

function RenderUI()
end