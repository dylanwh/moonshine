--local util = require "moonshine.object.util"

local Object = {}

function Object.clone(old)
	local new = {}
	for k, v in pairs(old) do
		new[k] = v
	end

	return setmetatable(new, getmetatable(old))
end

function Object.__index(self, key)
	local mt = getmetatable(self)
	if mt then
		return mt[key]
	end
end

function Object.new(mt, attr)
	local self = {}

	if getmetatable(mt) then
		mt = getmetatable(mt)
	end

	if attr then
		for k, v in pairs(attr) do
			self[k] = v
		end
	end

	setmetatable(self, mt)
	if self.__init then
		self:__init()
	end

	return self
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
