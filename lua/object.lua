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

Dog = Object:new()

function Dog:bark(a, b)
	print ("self = ", self)
	print ("a = ", a)
	print ("b = ", b)
end

dog = Dog:new()

f = dog:callback ("bark")
f(1,2)

