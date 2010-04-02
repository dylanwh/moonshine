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
    self._topic  = new "moonshine.ui.label"
    self._buffer = new "moonshine.ui.buffer"

    self._name     = assert(init.name, "name parameter is required")
    self._activity = 0
    self._is_focused  = false
    self:update_topic( "view: " .. self._name )
end

function View:update_topic(text)
    self._topic:set( format.apply('topic', text) )
end

function View:focus()
    self:clear_activity()
    self._is_focused = true
end

function View:unfocus()
    self._is_focused = false
end

function View:set_index(i)
    self._index = i
end

function View:get_index()
    return self._index or '?'
end

function View:get_name()
    return self._name or 'unnamed'
end

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

function View:set_activity(level)
    if not self._is_focused then
        self._activity = math.max(self._activity, level)
    end
end

function View:clear_activity()
    self._activity = 0
end

function View:add_message(msg)
    self:set_activity( msg.level or 1 )
    self._buffer:print(
        format.apply(msg.name, unpack(msg.args))
    )
end

function View:print(text, ...)
    self:set_activity(1)
    self._buffer:print(format.eval(text, ...))
end

function View:render(t, b)
    self._topic:render(t)
    self._buffer:render(t+1, b)
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
