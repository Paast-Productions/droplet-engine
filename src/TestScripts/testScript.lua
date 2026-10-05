function OnStart()
	print("Axel force me to smell his smelly feets :(")
end

function OnUpdate(dt)
	local flaot = Input:GetCursorX()

	if Input:KeyPressed(Key.Space) then
		print("Space is pressed")
	end

	if Input:MousePressed(Mouse.RMB) then
		print("RMB is pressed")
		print(flaot)
	end

	--local position = Vec3.new(100.0, 1.0, 90.0)
	--transform:SetPosition(position, TransformSpace.Local)
end

function RenderUI()
    assert(type(ImGui) == "table", "ImGui table is missing")
    assert(type(ImGui.Text) == "function", "ImGui.Text is missing")
    assert(type(ImGui.Button) == "function", "ImGui.Button is missing")
    assert(type(ImGui.Separator) == "function", "ImGui.Separator is missing")
    assert(type(ImGui.Spacing) == "function", "ImGui.Spacing is missing")
	assert(type(ImGui.DragFloat) == "function")
	assert(type(ImGui.DragInt) == "function")

    print("RenderUI call and ImGui bindings are working!")
end

