
--local ray = Ray.new()

function OnStart()
    local scene = SceneManagerInstance:GetScene("MainScene")

    if not scene then
        print("Scene not found")
        return
    end

    -- Replace these with a ray that passes through a known mesh.
    local origin = Vec3.new(0, 0, 0)
    local direction = Vec3.new(0, 0, 1)
    local ray = Ray.new(origin, direction)

    local hits = scene:RaycastNodes(ray)

    print("Raycast returned " .. #hits .. " nodes")

    for _, node in ipairs(hits) do
        print("Ray hit node: " .. node:GetName())
    end
end

function OnUpdate(dt)
end

function RenderUI()
end