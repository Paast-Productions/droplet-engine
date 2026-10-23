
local ray = Ray.new()
local scene = SceneManager:GetScene()

function OnStart()
    print(scene)
end

function OnUpdate(dt)
    print(ray.pos)
end

function RenderUI()

end
