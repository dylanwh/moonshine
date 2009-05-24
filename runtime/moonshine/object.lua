--local util = require "moonshine.object.util"

local Object = {}

function Object.__index(self, key)--{{{
	local mt = getmetatable(self)
	if mt then
		return mt[key]
	end
end--}}}

function Object.new(mt, attr)--{{{
	if getmetatable(mt) then
		error("metatables should not have metatables of their own.", 2)
	end

	local self = {}
	if attr then
		for k, v in pairs(attr) do
			self[k] = v
		end
	end

	setmetatable(self, mt)
	if self.__init then
		self:__init()
	end

	return self
end--}}}

function Object.clone(old_mt)--{{{
	if getmetatable(old_mt) then
		error("metatables should not have metatables of their own.", 2)
	end

	local new_mt = {}
	for k, v in pairs(old_mt) do
		new_mt[k] = v
	end
end--}}}

function Object:callback(name, ...)--{{{
	local cb_args = { ... }
	
	return function(...)
		local args = { ... }
		for i, arg in ipairs(cb_args) do
			table.insert(args, i, arg)
		end
		return self[name](self, unpack(args))
	end
end--}}}

return Object
