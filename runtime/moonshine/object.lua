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

local Object = { }
local mt     = { }

setmetatable(Object, mt)

function mt.__index(self, slot)
    local parent = rawget(self, '__parent')
    if parent then
        return parent[slot]
    end
end

function Object.new(parent, ...)
    local self = setmetatable( { __parent = parent }, getmetatable(parent) )

    local init = self.__init
    if init then init(self, ...) end

    return self
end

function Object:new_mt()
    local new_mt = {}
    for k, v in pairs(getmetatable(self)) do
        new_mt[k] = v
    end

    setmetatable(self, new_mt)

    return new_mt
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

return Object
