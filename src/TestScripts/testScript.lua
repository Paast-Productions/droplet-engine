function OnStart()
	print("Axel smells good for once")
end

function OnUpdate(dt)
	local transform = self:GetTransform()
	local position = Vec3.new(dt, 1.0, 90.0)
	transform:SetPosition(position, TransformSpace.Local)

	print(position.x)
end