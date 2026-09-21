
function onUpdate(dt)
	self:set_position(
		self:get_x() + 1.0 *dt,
		self:get_y(),
		self:get_z()
	)
	print("Gupta updating")
end


function onStart()
	print("Gupta says hello")
end

function backflip()
	print("Gupta does epic backflip")
end

function fishflip(a,b,c)
	print("Gupta does fish flip")
	self:set_position(
		self:get_x()+a+b+c,
		self:get_y(),
		self:get_z()
	)
end
