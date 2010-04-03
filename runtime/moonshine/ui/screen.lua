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

local term   = require "moonshine.ui.term"
local format = require "moonshine.ui.format"
local idle   = require "moonshine.idle"
local log    = require "moonshine.log"

local M = {}
local P = {}

function M.init()
    P.view    = new("moonshine.ui.view", { name = "debug" })
    P.status  = new "moonshine.ui.label"
    P.entry   = new "moonshine.ui.entry"
    P.history = new "moonshine.ui.buffer"
    P.views   = { P.view }

    P.view:set_index(1)
    P.view:focus()
    P.status_timer = new("moonshine.timer", function()
        M.render()
        return false
    end)
end

function M.print(...) P.view:print(...) end
function M.current_view() return P.view end

function M.add_view(view)
    local i = #P.views + 1
    P.views[i] = view
    view:set_index(i)
    return i
end

function M.find_view(i)
    return P.views[i]
end

function M.focus_view(i)
    local view = M.find_view(i)
    if view then
        P.view:unfocus()
        view:focus()
        P.view = view
    end
end

function M.is_focused(i)
    return P.view == M.find_view(i)
end

function M.view_info(key)
    return tostring(P.view:info(key))
end

function M.activity()
    local list = {}
    for i, view in ipairs(P.views) do
        local level = view:get_activity()
        if level then
            list[#list+1] = { index = i, level = level }
        end
    end

    return ipairs(list)
end

function M.update_status()
    local curr = os.date("*t")
    local next = {
        year  = curr.year,
        month = curr.month,
        day   = curr.day,
        hour  = curr.hour,
        min   = curr.min + 1,
        sec   = 0,
        isdst = curr.isdst
    }

    local wait = os.difftime(os.time(next), os.time(curr))
    wait = wait * 1000

    -- Since lua's time is only accurate to the second, undershoot by 1s
    -- to make sure we redraw closer to the actual minute rollover.
    -- The edge case handling below will then have us poll every 100ms
    -- until the minute rolls over.
    wait = wait - 1000

    if wait <= 0 or wait > 59 * 1000 then
        -- We've hit some kind of weird edge case. Try again in 100ms
        wait = 100
    end
    P.status_timer:schedule(wait)

    P.status:set( format.apply('status') )
end

function M.history_save()
    if not P.entry:is_dirty() or P.entry:get() == "" then
        return
    end

    -- Suppress repeated lines from going into history
    if P.history:at_end() and P.entry:get() == P.history:get_current() then
        return
    end
    P.history:print(P.entry:get())
end

function M.history_backward()
    M.history_save()
    -- Are we just starting to scroll back?
    if not P.history_at_end then
        P.history:scroll(1)
    end

    P.history_at_end = false
    local v = P.history:get_current()
    if v then
        P.entry:set(v)
    end
end

function M.history_forward()
    M.history_save()
    if P.history:at_end() then
        P.entry:clear()
        P.history_at_end = true
    else
        P.history:scroll(-1)
        local v = P.history:get_current()
        if v then
            P.entry:set(v)
        end
    end
end


function M.render()
    if P.render_pending then
        return
    end

    P.render_pending = true

    idle.call(function()
        local rows, cols = term.dimensions()

        M.update_status()

        P.view._buffer:is_dirty(true)
        P.view:render(0, rows - 3)
        P.status:render(rows - 2)
        P.entry:render( format.apply('prompt') )

        term.refresh()

        P.render_pending = false
    end)
end

function M.resize()
    term.resize()
    M.render()
end

function M.entry_keypress(key) P.entry:keypress(key) end
function M.entry_erase(...)    P.entry:erase(...)    end
function M.entry_move(...)     P.entry:move(...)     end
function M.entry_move_to(...)  P.entry:move_to(...)  end
function M.view_scroll(x)      P.view:scroll(x)      end

function M.entry_word_delete()
    local wlback, wlfwd = P.entry:wordlen()
    P.entry:erase(-wlback)
end

function M.entry_word_left()
    local wlback, wlfwd = P.entry:wordlen()
    P.entry:move(-wlback)
end

function M.entry_word_right()
    local wlback, wlfwd = P.entry:wordlen()
    P.entry:move(wlfwd)
end

-- pay it forward.
function M.entry_submit(f)
    assert(f, "submit requires a function to call")
    local line = P.entry:get()
    if #line > 0 and f then
        M.history_save()
        P.history:scroll_to(0)
        P.history_at_end = true
        P.entry:clear()
        M.render()
        f(line)
    end
end

return M
