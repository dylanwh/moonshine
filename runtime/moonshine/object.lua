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
	if #args == 0 then
		return function (...) self[name](self, ...) end
	elseif #args == 1 then
		return function (...) self[name](self, args[1], ...) end
	else
		return function (...)
			local xs = { unpack(args) }
			for _, x in ipairs { ... } do
				table.insert(xs, x)
			end
			return self[name](self, unpack(xs))
		end
	end
end
