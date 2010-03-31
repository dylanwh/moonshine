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

-- maps between limited integers and strings.
local Map = new "moonshine.object"

function Map:__init(max)
    self._to_id   = {}
    self._to_name = {}
    self._max   = max
end

function Map:assign(name, id)
    if id == nil then
        id = self:next_id()
    end

    assert(name ~= nil, "name must not be nil")
    assert(id <= self._max, "cannot assign id larger than " .. self._max)

    self._to_id[name] = id
    self._to_name[id] = name
    return id
end

function Map:find(name)
    return self._to_id[name]
end

function Map:find_or_assign(name)
    return self:find(name) or self:assign(name)
end

function Map:next_id()
    return #self._to_name + 1
end

return Map
