basetype = type

function type(x)--{{{
	local mt = getmetatable(x)
	if mt and mt.__type then
		return mt.__type
	else
		return basetype(x)
	end
end--}}}

function string:split(pat)--{{{
  local st, g = 1, self:gmatch("()("..pat..")")
  local function getter(self, segs, seps, sep, cap1, ...)
    st = sep and seps + #sep
    return self:sub(segs, (seps or 0) - 1), cap1 or sep, ...
  end
  local function splitter(self)
    if st then return getter(self, st, g()) end
  end
  return splitter, self
end--}}}

function collect(f, ...)--{{{
	local list = {}
	for x in f(...) do
		table.insert(list, x)
	end
	return list
end--}}}

function split(pat, str)--{{{
	return collect(string.split, str, pat)
end--}}}

--[[do
	local event = require "moonshine.event"
	function print(...)
		event.emit("print", ...)
	end
end]]

function each(x)
	if type(x) == 'table' then
		return ipairs(x)
	elseif type(x) == 'string' then
		local f = x:gmatch(".")
		local i = 0
		return function()
			i = i + 1
			local v = f()
			if v then
				return i, v
			end
		end
	else
		local mt = getmetatable(x)
		if mt and mt.__each then
			return mt.__each(x)
		else
			error("cannot each() over this object.")
		end
	end
end

function magic_table(canonize)
	local t  = {}
	local mt = {}
	setmetatable(t, mt)

	assert(canonize, "magic_table needs canonize function!")

	function mt.__index(t, key)
		return rawget(t, canonize(key))
	end

	function mt.__newindex(t, key, val)
		return rawset(t, canonize(key), val)
	end

	return t
end


local t = magic_table(string.lower)
t.NaME = "foo"
for k, v in pairs(t) do
	print (k, v)
end
