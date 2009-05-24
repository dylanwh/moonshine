local api = require "moonshine.object.api"

local Object = {}

api.init_class(Object)

function Object.__index(self, key)
	local class = api.getclass(self)
	if class then
		return class[key]
	end
end

function Object.new(class, ...)
	local self = api.new_object(class, ...)

	if self.__init then
		self:__init()
	end

	return self
end

function Object.clone(class)
	local new_class = {}
	for k, v in pairs(class) do
		new_class[k] = v
	end

	return api.init_class(new_class)
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
