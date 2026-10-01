function OnStart()
	print("Axel smells good (not too sure)")
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

