
--local ray = Ray.new()

local scene

function OnStart()
    scene = SceneManagerInstance:GetScene("MainScene")

    if not scene then
        print("Scene not found")
        return
    end

    local transform = self:GetTransform()

    local origin = transform:GetPosition(TSWorld)
    local direction = transform:GetForward(TSWorld)

    local ray = Ray.new(origin, direction)
    local hits = scene:RaycastNodes(ray)

    print("Raycast returned " .. #hits .. " nodes")

    for _, node in ipairs(hits) do
        if node ~= self then
            print("Ray hit node: " .. node:GetName())
        end
    end
end

function OnUpdate(dt)

    local transform = self:GetTransform()
    local origin = transform:GetPosition(TSWorld)
    local direction = transform:GetForward(TSWorld)

    local ray = Ray(origin, direction)
    local node, hit = scene:RaycastNode(ray)

    if node and hit.didHit then
        print("Hit: " .. node:GetName())
        print("Distance: " .. hit.distance)
    end
end

function RenderUI()
end