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

local format = require "moonshine.ui.format"

local View  = new "moonshine.object"

function View:__init(init)
    self._topic      = new "moonshine.ui.label"
    self._buffer     = new "moonshine.ui.buffer"
    self._name       = assert(init.name, "name parameter is required")
    self._activity   = 0
    self._is_focused = false

    self:update_topic( "view: " .. self._name )
end

function View:update_topic(text)
    self._topic:set( format.apply('topic', text) )
end

function View:focus()
    self._is_focused = true
    self:clear_activity()
end

function View:unfocus()
    self._is_focused = false
end

-- by default, view 1 cannot be closed.
function View:can_close() return self._index and not self._index == 1 end

-- the screen calls this method when the view is closed.
function View:close()
    self:set_index(nil)
    self:unfocus()
end

function View:set_index(i)   self._index = i    end
function View:get_index()    return self._index end

function View:set_name(name) self._name = name  end
function View:get_name()     return self._name  end

function View:set_activity(level)
    if not self._is_focused then
        self._activity = math.max(self._activity, level)
    end
end

function View:clear_activity() self._activity = 0 end

function View:get_activity()
    if self._activity > 2 then
        return 'important'
    elseif self._activity == 2 then
        return 'normal'
    elseif self._activity == 1 then
        return 'boring'
    else
        return nil
    end
end

function View:add_message(msg)
    self:set_activity( msg.level or 1 )
    self:_buffer_print(
        format.apply(msg.name, unpack(msg.args))
    )
end

function View:print(text, ...)
    self:set_activity(1)
    self:_buffer_print(format.eval(text, ...))
end

local function userlist_tabularize(users)
    local T = #users
    local C = 5 -- FIXME: irssi picks a number from 1 to $CAP based on terminal width.
    local R = math.ceil(T/C)

    local function f(x) return 1+(x-1)%R, math.ceil(x/R) end

    local data = {}
    local max  = {}
    for i, user in ipairs(users) do
        local r, c = f(i)
        if not data[r] then
            data[r] = {}
        end
        if not max[c] or max[c] < #user.name then
            max[c] = #user.name
        end
        data[r][c] = user
    end

    return data, max
end

function View:show_userlist(room, users)
    local ops, halfops, voices, normals = 0,0,0,0
    for _, user in ipairs(users) do
        if user.flags.op then
            ops = ops + 1
        elseif user.flags.halfop then
            halfops = halfops + 1
        elseif user.flags.voice then
            voices = voices + 1
        else
            normals = normals + 1
        end
    end

    local total = ops + halfops + voices + normals
    self:_buffer_print( format.apply("userlist_head", room) )

    local tab, max = userlist_tabularize(users)
    for _, line in ipairs(tab) do
        for i, item in ipairs(line) do
            local name = string.format("%-" .. max[i] .. "s", item.name)
            line[i] = format.apply('userlist_item', item.flags, name)
        end
        self:_buffer_print( format.apply('userlist_line', unpack(line)) )
    end
    self:_buffer_print( format.apply("userlist_foot", room, total, ops, halfops, voices, normals) )

end

function View:_buffer_print(line)
    self._buffer:print(line:gsub("\n", " ") )
end

function View:render(t, b)
    self._topic:render(t)
    self._buffer:render(t+1, b)
end

function View:scroll(x)
    self._buffer:scroll(x)
end

function View:info(key)
    if key == 'name' then
        return self:get_name()
    elseif key == 'index' then
        return self:get_index()
    else
        return 'info:' .. key
    end
end

return View
