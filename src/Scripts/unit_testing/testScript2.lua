function OnStart()
	print("Started Lua script")
end	

function onUpdate(dt)
		self:set_position(
		self:get_x() - 2.0 *dt,
		self:get_y(),
		self:get_z()
	)
end

function test()
	print("lua function")
end

function Lua_Sum(a,b)
	return a+b
end

function Lua_Backflip()
	print("Gupta does cool backflip")
end