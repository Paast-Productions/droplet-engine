
--local ray = Ray.new()

function OnStart()
    print("fish interaction start")
    local scene = SceneManagerInstance:GetScene("MainScene")

    if scene then
        print("MainScene")
    end
    print(scene)
end

function OnUpdate(dt)


end

function RenderUI()
end