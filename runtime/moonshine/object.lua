local Object = {}

function Object:clone (...)
	local t = {}
	local mt  = { __index = self }
	setmetatable(t, mt)
	t:init(...)
	return t
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
		return function (...) return self[name](self, ...) end
	elseif #args == 1 then
		return function (...) return self[name](self, args[1], ...) end
	elseif #args == 2 then
		return function (...) return self[name](self, args[1], args[2], ...) end
	elseif #args == 3 then
		return function (...) return self[name](self, args[1], args[2], args[3], ...) end
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
