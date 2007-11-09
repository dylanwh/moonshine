Object = {}

function Object:new (obj)
	obj = obj or {}
  	setmetatable(obj, self)
  	self.__index = self
  	if obj.init then
  		obj:init()
  	end
  	return obj
end

function Object:callback(name, ...)
	local args = arg
	return function (...)
		if #args > 0 then
			return self[name](self, unpack(args), ...)
		else
			return self[name](self, ...)
		end
	end
end
