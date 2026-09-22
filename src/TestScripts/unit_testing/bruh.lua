function onStart()
	print("Started Lua script")
end	

local jonas = 0

function onUpdate(dt)
	jonas = jonas + dt
end

function timesTwo(a)
	return a * 2
end

function getDT()
	return jonas
end
