--[[ vim: set ft=lua sw=4 ts=4 expandtab:
-   Moonshine - a Lua-based chat client
-
-   Copyright (C) 2010 Dylan William Hardison
-
-   This file is part of Moonshine.
-
-   Moonshine is free software: you can redistribute it and/or modify
-   it under the terms of the GNU General Public License as published by
-   the Free Software Foundation, either version 3 of the License, or
-   (at your option) any later version.
-
-   Moonshine is distributed in the hope that it will be useful,
-   but WITHOUT ANY WARRANTY; without even the implied warranty of
-   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-   GNU General Public License for more details.
-
-   You should have received a copy of the GNU General Public License
-   along with Moonshine.  If not, see <http://www.gnu.org/licenses/>.
]]

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

function new(mod, ...)
    local class = require(mod)
    assert(type(class) == 'table' or type(class) == 'userdata', "module " .. mod .. " did not return object")
    assert(class.new, "module " .. mod .. " does not support :new()!")
    return class:new(...)
end

function map(f, list, ...)
    local result = {}
    if not list then
        return result
    end
    for i, x in ipairs(list) do
        result[i] = f(x, ...)
    end
    return result
end

function grep(f, list, ...)
    local result = {}
    if not list then return result end
    local i = 1
    for _, x in ipairs(list) do
        if f(x, ...) then
            result[i] = x
            i = i + 1
        end
    end
end

function default(t, val)
    local mt = { __index = function() return val end }
    return setmetatable(t, mt)
end

