local getmetatable = getmetatable
local setmetatable = setmetatable
local table        = table
local unpack       = unpack
local ipairs       = ipairs

module "moonshine.object"

__type = 'Object'

function clone (self, obj)
	obj = obj or {}
  	setmetatable(obj, self)
  	self.__index = self
  	if obj.init then
  		obj:init()
  	end
  	return obj
end

function parent(self)
	return getmetatable(self)
end

function callback(self, name, ...)
	local args = {...}
	return function (...)
		if #args == 0 then
			return self[name](self, ...)
		elseif #args == 1 then
			return self[name](self, args[1], ...)
		elseif #args > 1 then
			local nargs = { unpack(args) }
			for i, v in ipairs { ... } do
				table.insert(nargs, v)
			end
			return self[name](self, unpack(nargs))
		end
	end
end
