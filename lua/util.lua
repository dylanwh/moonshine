basetype = type
function type(x)
	local t = basetype(x)
	if t == 'userdata' or t == 'table' then
		return x.__type or t
	else
		return t
	end
end

function assert(cond, msg)
	if not cond then
		if not msg then
			msg = "assertion failed!"
		end
		if type(msg) == 'Error' then
			msg = msg.message
		else
			msg = tostring(msg)
		end

		error(msg, 2)
	end
end

function weaktable()
	local t = {}
	local mt = { __mode = "v" }
	setmetatable(t, mt)
	return t
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
