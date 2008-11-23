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

do
	local signal = require "moonshine.signal"
	function print(...)
		signal.emit("print", ...)
	end
end
