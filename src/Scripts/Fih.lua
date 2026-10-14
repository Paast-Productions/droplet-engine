
function OnStart()
	print("New Fih")
end

function OnUpdate(dt)
	
	local moveSpeed = 10.0

	local movement = Vec3.new(0.0, 0.0, 0.0)
	local rotation = 0.0
	local doMove = false

	if Input:KeyHeld(Key.ArrowUp) then
		movement.z = movement.z + dt
		doMove = true
	end

	if Input:KeyHeld(Key.ArrowDown) then
		movement.z = movement.z - dt
		doMove = true
	end

	if Input:KeyHeld(Key.ArrowLeft) then
		rotation = rotation - dt
		doMove = true
	end

	if Input:KeyHeld(Key.ArrowRight) then
		rotation = rotation + dt
		doMove = true
	end

	if doMove then
		movement.y = movement.y * moveSpeed * 1.0
		rotation = rotation * moveSpeed * 0.1

		local trans = self:GetTransform()
		trans:MoveRelative(movement, TransformSpace.Local)
		trans:RotateEulerRelative(Vec3.new(0.0, rotation, 0.0), TransformSpace.World)
		print("Move Fih")
	end

end

function RenderUI()
	
end

