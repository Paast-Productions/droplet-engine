function onStart()
	print("Started Lua script")
end	

function onUpdate(dt)
		self:set_position(
		self:get_x() - 2.0 *dt,
		self:get_y(),
		self:get_z()
	)
end