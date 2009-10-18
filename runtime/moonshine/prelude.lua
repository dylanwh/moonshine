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

function join(sep, list)--{{{
    if #list == 0 then
        return ''
    elseif #list == 1 then
        return tostring(list[1])
    end

    for i = 1, #list do
        list[i] = tostring(list[i])
    end
    return table.concat(list, sep or '')
end--}}}

string.join = join

function accessor(slot)
    return function(self, ...)
        if select('#', ...) == 0 then
            return self[slot]
        else
            self[slot] = ...
            return self[slot]
        end
    end
end

function new(mod, ...)
    local class = require(mod)
    return class:new(...)
end
