
local interactionRange = 2.5
local interactionAngle = 5.0

local scene
local debugDrawEnabled = false

local function Normalize(v)
    local length = math.sqrt(
        v.x * v.x +
        v.y * v.y +
        v.z * v.z
    )

    if length < 0.0001 then
        return v
    end

    return Vec3.new(
        v.x / length,
        v.y / length,
        v.z / length
    )
end

local function GetRayDirections()
    local transform = self:GetTransform()

    local forward = transform:GetForward(TransformSpace.World)
    local right = transform:GetRight(TransformSpace.World)
    local up = transform:GetUp(TransformSpace.World)

    local offset = math.tan(math.rad(interactionAngle))

    return {
        Normalize(forward),

        Normalize(Vec3.new(
            forward.x + right.x * offset,
            forward.y + right.y * offset,
            forward.z + right.z * offset
        )),

        Normalize(Vec3.new(
            forward.x - right.x * offset,
            forward.y - right.y * offset,
            forward.z - right.z * offset
        )),

        Normalize(Vec3.new(
            forward.x + up.x * offset,
            forward.y + up.y * offset,
            forward.z + up.z * offset
        )),

        Normalize(Vec3.new(
            forward.x - up.x * offset,
            forward.y - up.y * offset,
            forward.z - up.z * offset
        ))
    }
end

local function GetInteractionTarget()
    local origin = self:GetTransform():GetPosition(TransformSpace.World)
    local directions = GetRayDirections()

    local bestTarget = nil
    local bestDistance = interactionRange

    for _, direction in ipairs(directions) do
        local ray = Ray.new(origin, direction)
        local targets = scene:RaycastNodes(ray)

        for _, target in ipairs(targets) do
            if target ~= nil and target:IsActive() then
                local interactable = target:GetInteractableComponent()

                if interactable ~= nil then
                    local position = target:GetTransform():GetPosition(TransformSpace.World)

                    local dx = position.x - origin.x
                    local dy = position.y - origin.y
                    local dz = position.z - origin.z

                    local distance = math.sqrt(
                        dx * dx +
                        dy * dy +
                        dz * dz
                    )

                    if distance <= bestDistance then
                        bestTarget = target
                        bestDistance = distance
                    end
                end
            end
        end
    end

    return bestTarget
end

function OnStart()
    scene = self:GetScene()
end

function OnUpdate(dt)
    -- Toggle debug drawing.
    if Input:KeyPressed(Key.B) then
        debugDrawEnabled = not debugDrawEnabled
        print("Interaction debug draw:", debugDrawEnabled)
    end

    local origin = self:GetTransform():GetPosition(TransformSpace.World)
    local directions = GetRayDirections()

    if debugDrawEnabled then
        for _, direction in ipairs(directions) do
            local ray = Ray.new(origin, direction)

            DebugDraw.DrawRay(
                ray,
                Vec4.new(1, 0, 0, 1),
                false
            )
        end
    end

    if not Input:KeyPressed(Key.E) then
        return
    end

    local target = GetInteractionTarget()

    if target ~= nil then
        local interactable = target:GetInteractableComponent()

        if interactable ~= nil then
            interactable:Interact(self)
        end
    end
end

function RenderUI()
    _, interactionRange = ImGui.SliderFloat(
        "Interaction Range", interactionRange, 0.5, 100.0
    )

    _, interactionAngle = ImGui.SliderFloat(
        "Interaction Angle", interactionAngle, 1.0, 60.0
    )

    ImGui.Text("Debug Draw (B): " .. tostring(debugDrawEnabled))
end
