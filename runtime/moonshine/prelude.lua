term   = require "moonshine.ui.term"
keymap = require "moonshine.ui.keymap"
Screen = require "moonshine.ui.screen"
screen = nil

basetype = type
function type(x)
	local t = basetype(x)
	if t == 'userdata' or t == 'table' then
		return x.__type or t
	else
		return t
	end
end

function string:split(pat)
  local st, g = 1, self:gmatch("()("..pat..")")
  local function getter(self, segs, seps, sep, cap1, ...)
    st = sep and seps + #sep
    return self:sub(segs, (seps or 0) - 1), cap1 or sep, ...
  end
  local function splitter(self)
    if st then return getter(self, st, g()) end
  end
  return splitter, self
end

function collect(f, ...)
	local list = {}
	for x in f(...) do
		table.insert(list, x)
	end
	return list
end

function join(sep, list)
	return table.concat(list, sep)
end

function split(pat,str)
	return collect(string.split, str, pat)
end

local atexit_funcs = {}

function atexit(f)
	table.insert(atexit_funcs, f)
end

function exit_hook()
	for _, f in ipairs(atexit) do
		f()
	end
end

function register(name, value)
	local t = _G
	local pt, pk
	for k in name:split("%.") do
		print(k, pk)
		if t[k] == nil then
			t[k] = {}
		end
		pt, pk  = t, k
		t       = t[k]
	end
	if pt[pk] == nil then
		pt[pk] = value
	else
		error("cannot register " .. name .. " (already defined)")
	end
end
