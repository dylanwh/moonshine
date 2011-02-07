--[[ vim: set ft=lua sw=4 ts=4 expandtab:
-   Moonshine - a Lua-based chat client
-
-   Copyright (C) 2010 Dylan William Hardison, Bryan Donlan
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

local bit = require "bit"

local function ctrl(c)
    return string.char(bit.bxor(string.byte(string.upper(c)), 64))
end

local function keyspec(spec)
    local term = require "moonshine.ui.term"
    return spec:gsub("%^(.)", ctrl):gsub('{([%w_]+)}', term.tigetstr)
end

local Keymap = new "moonshine.object"

function Keymap:__init()
    self._tree   = new "moonshine.tree"
    self._keybuf = ""
end

function Keymap:process(key)
    local tree  = self._tree
    self._keybuf = self._keybuf .. key

    local found_key, func, index, dirn = tree:find_near(self._keybuf)
    if found_key == nil then
        -- No keys defined??
        return false
    end

    if dirn == 0 then
        self._keybuf = ""
        func()
        return true
    elseif dirn > 0 then
        index     = index + 1
        found_key = tree:lookup_index(index)
    end

    if found_key == nil then
        -- should only ever happen if there are no keys registered at all
        found_key = ""
    end

    if string.sub(found_key, 1, string.len(self._keybuf)) ~= self._keybuf then
        -- not a prefix of anything
        self._keybuf = ""
        return false
    else
        -- prefix of something, keep going
        return true
    end
end

function Keymap:bind(spec, func)
    if spec == nil then
        return
    end
    self._tree:insert(keyspec(spec), func)
end

return Keymap
