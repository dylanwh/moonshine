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

local purple_core          = require "purple.core"
local purple_conversations = require "purple.conversations"
local purple_roomlist      = require "purple.roomlist"

local log    = require "moonshine.log"
local term   = require "moonshine.ui.term"
local shell  = require "moonshine.shell"
local keymap = new "moonshine.keymap"

local format = require "moonshine.ui.format"
local screen = require "moonshine.ui.screen"

local TO_VIEW = { }

function on_input(key)
    if not keymap:process(key) then
        screen.entry_keypress(key)
    end
    screen.render()
end

function on_resize()
    screen.resize()
end

-- we use only a single hash for all ui_ops (for now),
-- the reason for this is just laziness.
local H = {}

function H.ui_init()
    purple_conversations.init(H)
    purple_roomlist:init(H)
    format.init()
    screen.init()

    log.set_default_handler(function (domain, level, message)
        screen.find_view(1):add_message( { level = 1, name = "log_message", args = { domain, level, message } } )
        screen.render()
    end)

    -- backspace:
    keymap:bind('{kbs}',   function () screen.entry_erase(-1)    end)
    -- left, right, up, down:
    keymap:bind('{kcub1}', function () screen.entry_move(-1)     end)
    keymap:bind('{kcuf1}', function () screen.entry_move(1)      end)
    keymap:bind('{kcuu1}', function () screen.history_backward() end)
    keymap:bind('{kcud1}', function () screen.history_forward()  end)
    -- shift+left, shift+right
    keymap:bind('{kLFT}',  screen.entry_word_left)
    keymap:bind('{kRIT}',  screen.entry_word_right)
    -- home/end
    keymap:bind('{khome}', function () screen.entry_move_to(0)   end)
    keymap:bind('{kend}',  function () screen.entry_move_to(-1)  end)
    -- kpp = page up, knp = page down
    keymap:bind('{kpp}',   function () screen.view_scroll(5)     end)
    keymap:bind('{knp}',   function () screen.view_scroll(-5)    end)

    for i = 1, 9 do
        keymap:bind('^[' .. i, function () screen.focus_view(i) end)
    end
    keymap:bind('^[0', function () screen.focus_view(10) end)

    local function accept(text)
        shell.execute(text)
        screen.render()
    end

    keymap:bind('{kent}',  function () screen.entry_submit(accept) end)
    -- why does urxvt not sent kent?
    keymap:bind('^M',  function () screen.entry_submit(accept) end)

    keymap:bind('^W', function () screen.entry_word_delete() end)
    keymap:bind('^A', function () screen.entry_move_to(0) end)
    keymap:bind('^E', function () screen.entry_move_to(-1) end)

    screen.render()
end

function H.conversation_create(conv)
    assert(TO_VIEW[conv] == nil, "conversation does not exist")
    local view = new("moonshine.ui.view.conversation", { name = conv:get_name(), conversation = conv })
    TO_VIEW[conv] = screen.add_view(view)
    screen.print("new conv: $1", TO_VIEW[conv])
    screen.render()
end

function H.conversation_destroy(conv)
    assert(TO_VIEW[conv], "conversation exists")
    screen.del_view( TO_VIEW[conv] )
    screen.render()
end

function H.conversation_write_im(conv, name,  message, flags, mtime)
    assert(TO_VIEW[conv], "conversation exists")
    local view = screen.find_view(TO_VIEW[conv])
    if name then
        view:add_message({
            level = 3,
            name  = 'private',
            args  = { mtime, name, message },
        })
    else
        -- message from self?
        local account = conv:get_account()
        local name    = account:get_username()
        if account:get_protocol_id() == "prpl-irc" then
            name = name:match("(.+?)@")
        end
        view:add_message({
            level = 0,
            name  = "private_sent",
            args  = { mtime, name, message },
        })
    end

    screen.render()
end

function H.conversation_write_chat(conv, name, message, flags, mtime)
    assert(TO_VIEW[conv], "conversation exists")
    local view = screen.find_view(TO_VIEW[conv])
    view:add_message({
        level = 2,
        name  = 'public',
        args  = {mtime, name, message},
    })
    screen.render()
end

function H.conversation_present(conv)
    assert(TO_VIEW[conv], "conversation exists")
    screen.focus_view(TO_VIEW[conv])
    screen.render()
end

function H.conversation_has_focus(conv)
    assert(TO_VIEW[conv], "conversation exists")
    screen.render()
    return screen.is_focused(TO_VIEW[conv])
end

function H.roomlist_create(roomlist)
    log.debug("roomlist created")
    ROOMLIST = roomlist
    ROOM     = {}
end

function H.roomlist_add_room(roomlist, room)
    log.debug("room: %s", room:get_name())
    if not ROOM then
        ROOM = {}
    end
    ROOM[ room:get_name() ] = room
end

function H.roomlist_in_progress(roomlist, bool)
    if bool then
        log.debug("roomlist in progress")
    else
        log.debug("roomlist not in progress")
    end
end

local M = {}

function M.init()
    term.init()
    purple_core.init(H)
end

return M
