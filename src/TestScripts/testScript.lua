function OnStart()
	print("Axel smells good for once")
end

function OnUpdate(dt)
	--print("yes")
	local transform = self:GetTransform()
	local position = transform:IsDirty()

	print(position)
end