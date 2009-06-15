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

function join(sep, list)
	if #list == 0 then
		return ''
	elseif #list == 1 then
		return tostring(list[1])
	end

	for i = 1, #list do
		list[i] = tostring(list[i])
	end
	return table.concat(list, sep or '')
end

function each(x)--{{{
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
end--}}}

function magic_table(canonize)--{{{
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
end--}}}

local hook_mt = {
	__call = function (hook, ...)
		if hook.pred then
			assert(hook.pred(...))
		end

		for _, func in ipairs(hook) do
			func(...)
		end
	end
}

local function get_hook(name)
	return _G[ "on_" .. name:gsub(" ", "_") ]
end

local function set_hook(name, hook)
	_G[ "on_" .. name:gsub(" ", "_") ] = hook
end

function def_hook(name, check)
	local old_hook = get_hook(name)
	local new_hook = { check = check }

	if not old_hook or type(old_hook) == 'function' then
		table.insert(new_hook, old_hook)
	else
		return old_hook
	end

	set_hook(name, setmetatable(new_hook, hook_mt))
	return new_hook
end

function add_hook(name, func)
	local hook = get_hook(name)
	if not hook or type(hook) == 'function' then
		hook = def_hook(name)
	end
	table.insert(hook, 1, func)
end

function run_hook(name, ...)
	local hook = get_hook(name)
	if hook then
		return hook(...)
	elseif on_unknown_hook then
		return on_unknown_hook(name, { ... } )
	end
end


