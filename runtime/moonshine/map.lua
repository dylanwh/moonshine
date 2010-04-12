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

function Map:__init(limit)
    self._names = {}
    self._refs  = {}
    self._limit = limit
end

function Map:_ref(id)
    self._refs[id] = (self._refs[id] or 0) + 1
end

function Map:_unref(id)
    local c = self._refs[id] - 1
    if c == 0 then
        -- no longer referenced
        c = nil
    end
    self._refs[id] = c
end

function Map:_next_id()
    return #self._refs + 1
end

function Map:assign(name, id)
    if id == nil then
        id = self:_next_id()
    end

    assert(name ~= nil, "name must not be nil")
    assert(id <= self._limit, "cannot assign id larger than " .. self._limit)

    self:_ref(id, name)

    --if self._names[name] then
    --    self:_unref( self._names[name] )
    --end

    self._names[name] = id

    return id
end

function Map:find(name)
    return self._names[name]
end

function Map:find_or_assign(name)
    return self:find(name) or self:assign(name)
end

function Map:data()
    return {
        refs = self._refs,
        limit = self._limit,
        names = self._names,
    }
end

return Map
