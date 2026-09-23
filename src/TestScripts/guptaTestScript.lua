function onUpdate(dt)
	self:set_position(
		self:get_x() + 2.0 *dt,
		self:get_y(),
		self:get_z()
	)
	print("Gupta updating")
end

function onStart()
	print(TestDefinitionTest)
end

function backflip()
	print("Gupta does epic backflip")
end

function fishflip(a,b,c)
	print("Toe does a frontflip")
	self:set_position()
end
