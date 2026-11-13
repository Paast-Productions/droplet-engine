
local fishSystem = require("FishSystem")

-- Called when script starts
function OnStart()
    print("UpdgradeItem", self)
    fishSystem.RegisterFish(self, "speed")
end

-- Called every frame
function OnUpdate(dt)

end

-- Called to render variables in the script
function RenderUI()

end
