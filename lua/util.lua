

function join(sep, list)
	return table.concat(list, sep)
end

string.join = join

function split(div,str)
    if (div=='') then return false end
    local pos,arr = 0,{}
    local function iter()
    	return string.find(str,div,pos,true)
    end 
    -- for each divider found
    for st, sp in iter do
    	table.insert(arr,string.sub(str,pos,st-1)) -- Attach chars left of current divider
    	pos = sp + 1 -- Jump past current divider
	end
	table.insert(arr,string.sub(str,pos)) -- Attach chars right of last divider
	return arr
end

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
