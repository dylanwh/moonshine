--local util = require "moonshine.object.util"

local Object = {}

local OPS = { "__add", "__sub", "__div", "__mul", "__eq", "__call" }

function Object.new(parent, attr)
	local mt   = { parent = parent, __index = parent }
	local self = setmetatable({}, mt)

	if attr then
		assert(type(attr) == 'table', "new(attr): attr is table")
		for k, v in pairs(attr) do self[k] = v end
	end
	for i, name in ipairs(OPS) do
		if self[name] then
			mt[name] = self[name]
		end
	end
	--[[if self.__index then
		function mt.__index(self, key)
			local val = mt.parent[key]
			if val == nil then
				return self:__index(key)
			else
				return val
			end
		end
	end]]

	self:__init()

	return self
end

function Object:__init()
end

function Object:callback(name, ...)
	local cb_args = { ... }
	
	return function(...)
		local args = { ... }
		for i, arg in ipairs(cb_args) do
			table.insert(args, i, arg)
		end
		return self[name](self, unpack(args))
	end
end

return Object
