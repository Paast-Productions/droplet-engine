function OnStart()
	print("Axel smells ew today")
end

function OnUpdate(dt)
	local flaot = Input:GetCursorX()
	local position = Vec3.new(0.0, 0.0, 0.0)
	--local trans = self:GetTransform() 

	if Input:KeyPressed(Key.Space) then
		print("Space is pressed")
		trans:SetPosition(position, TransformSpace.Local)
	end

	if Input:MousePressed(Mouse.RMB) then
		print("RMB is pressed")
		print(flaot)
	end

	--print(Input:KeyPressed(Key.Space))
end             

