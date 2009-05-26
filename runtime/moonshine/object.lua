local Object = { __attributes = {} }

function Object.__index(self, key)
	local class = getmetatable(self)
	if class then
		return class[key]
	end
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

function Object.new(class, param)
	assert(getmetatable(class) == nil, "is class")

	local self = {}
	local attributes = class.__attributes or {}

	for name, option in pairs(attributes) do
		local slot = '!' .. name

		if param and param[name] then
			self[ slot ] = param[name]
		elseif option.default then
			self[ slot ] = option.default
		elseif option.required then
			error("required parameter missing: " .. name, level or 2)
		end
	end

	setmetatable(self, class)

	if self.__init then
		self:__init()
	end

	return self
end

function Object.add_attribute(class, name, option)
	assert(getmetatable(class) == nil, "is class")

	local slot = '!' .. name
	class.__attributes[name] = option
	class[name] = function(self, ...)
		local n = select('#', ...)
		if n == 0 then
			return self[slot]
		else
			self[slot] = ...
			return self[slot]
		end
	end

	if option.handles then
		for _, method in ipairs(option.handles) do
			class[method] = function (self, ...)
				local log  = require "moonshine.log"
				local obj  = self[slot]
				local func = obj[method]
				return func(obj, ...)
			end
		end
	end
end

function Object.subclass(class)
	assert(getmetatable(class) == nil, "is class")

	local new_class = {}
	for k, v in pairs(class) do
		new_class[k] = v
	end

	new_class.__attributes = {}
	for k, v in pairs(class.__attributes) do
		new_class.__attributes[k] = v
	end

	return new_class
end

return Object
