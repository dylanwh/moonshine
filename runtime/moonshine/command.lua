local mt = {}
function mt:__newindex(k, v)
	print("load", k)
	assert(v.parse, "commands must have parse function")
	assert(v.run, "commands must have run function")
	rawset(self, k, v)
	
end

local function f(M)
	setmetatable(M, mt)
end

module("moonshine.command", f)


