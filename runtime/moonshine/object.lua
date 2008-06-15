local Object = {}

function Object:clone (...)
	local mt  = { __index = self }
	local thing = {}
	setmetatable(thing, mt)
	print "running init on thing..."
	thing:init(...)
	return thing
end

function Object:init(table)
	if table then
		for k, v in pairs(table) do
			self[k] = v
		end
	end
end

function Object:parent()
	local mt = getmetatable(self)
	if mt then
		return mt.__index
	end
end

function Object:callback(name, ...)
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

return Object
