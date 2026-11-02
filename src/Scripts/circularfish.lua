local trans = nil

local orbitPoint = Vec3.new(4.0, 0.0, 0.0)
local orbitRadius = 3.0
local angularSpeed = 0.5 -- radians per sekund

-- Called when script starts
function OnStart()
    tracy.ZoneBeginN("Fih OnStart")

	print("New Fih")
	trans = self:GetTransform()
	local position = trans:GetPosition(TransformSpace.World)
    orbitPoint = GLM.V3Sub(
        position, Vec3.new(orbitRadius,0.0,0.0)
    )

    angle = 0.5

    tracy.ZoneEnd()
end

-- Called every frame
function OnUpdate(dt)
    angle = angle + angularSpeed *dt

    --anlge = angle % (2.0*math.pi)

    local offset = Vec3.new(math.cos(angle)*orbitRadius,0.0,math.sin(angle)*orbitRadius)

    local targetPosition = GLM.V3Add(orbitPoint,offset)

    print("Gupta update")
    local currentPosition =
        trans:GetPosition(TransformSpace.World)

    local movement = GLM.V3Sub(
        targetPosition,
        currentPosition
    )
    print(movement)
    trans:Move(movement, TransformSpace.World)

end

function RenderUI()

    _, orbitRadius = ImGui.DragFloat("orbit radius",orbitRadius, 0.1)
    _, angularSpeed = ImGui.DragFloat("Angular speed", angularSpeed, 0.05)
    _, orbitPoint = ImGui.DragFloat3("Orbit Point", orbitPoint,0.2)
end
