local M = {}

local class_mt = {
	__type = 'class',
}

function M.init_class(class)
	class.__attribute = {}
	return setmetatable(class, class_mt)
end

function M.is_class(class)
	local mt = getmetatable(class)
	return mt and mt.__type == 'class'
end

M.getclass = getmetatable

function M.setclass(obj, class)
	assert(M.is_class(class), "is class")

	return setmetatable(obj, class)
end

function M.getclass(obj)
	return getmetatable(obj)
end

function M.new_object(class, param, level)
	assert(M.is_class(class), 'is class')

	local self = {}
	local attr = class.__attribute

	for k, v in pairs(attr) do
		if param and param[k] then
			self[ v.slot ] = param[k]
		elseif v.default then
			self[ v.slot ] = v.default
		elseif v.required then
			error("required parameter missing: " .. k, level or 3)
		end
	end

	return M.setclass(self, class)
end

function M.add_attribute(class, name, attr)
	assert(M.is_class(class), "is class")

	local slot = '!' .. name
	attr.slot = slot
	class.__attribute[name] = attr
	class[name] = function(self, ...)
		local n = select('#', ...)
		if n == 0 then
			return self[slot]
		else
			self[slot] = ...
			return self[slot]
		end
	end
	if attr.handles then
		for _, method in ipairs(attr.handles) do
			class[method] = function (self, ...)
				return self[slot][method](self[slot], ...)
			end
		end
	end
end

return M
