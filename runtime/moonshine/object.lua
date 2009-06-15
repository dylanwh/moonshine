local Object = { __attributes = {} }

function Object:__index(key)
	local mt = getmetatable(self)
	if mt then
		return mt[key]
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

function Object.new(mt, param)
	assert(getmetatable(mt) == nil, "is mt")

	local self = setmetatable({}, mt)


	if param then
		for k, v in pairs(param) do
			if self[k] then
				self[k](self, v)
			end
		end
	end

	if self.__new then
		self:__new()
	end

	return self
end

function Object.clone(mt)
	assert(getmetatable(mt) == nil, "is mt")

	local new_mt = {}
	for k, v in pairs(mt) do
		new_mt[k] = v
	end

	if new_mt.__clone then
		new_mt:__clone()
	end
	return new_mt
end

function Object.add_attribute(mt, name)
	assert(getmetatable(mt) == nil, "is mt")

	local slot = '_' .. name

	mt[name] = function(self, ...)
		local n = select('#', ...)
		if n == 0 then
			return self[slot]
		else
			local v = ...
			self[slot] = v
			return v
		end
	end
end

return Object
