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
local purple_account       = require "purple.account"

local keymap = new "moonshine.keymap"
local log    = require "moonshine.log"
local term   = require "moonshine.ui.term"
local shell  = require "moonshine.shell"
local format = require "moonshine.ui.format"
local theme  = require "moonshine.ui.theme"
local screen = require "moonshine.ui.screen"

require "moonshine.ui.view"

local TO_VIEW = { }
local M = {}

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
    purple_account:init(H)
    format.init()
    theme.apply('default')
    screen.init()

    log.set_default_handler(function (domain, level, message)
        local f = io.open('moonshine.log', 'a')
        f:write(message .. "\n")
        f:close()
        screen.get_view(1):print_message( { level = 1, name = "log_message", args = { domain, level, message } } )
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
    -- why does urxvt not send kent?
    keymap:bind('^M',  function () screen.entry_submit(accept) end)

    keymap:bind('^W', function () screen.entry_word_delete() end)
    keymap:bind('^A', function () screen.entry_move_to(0) end)
    keymap:bind('^E', function () screen.entry_move_to(-1) end)
    keymap:bind('^C', function () quit() end)

    screen.render()
end

function H.conversation_create(conv)
    assert(TO_VIEW[conv] == nil, "conversation does not exist")
    local view    = new("moonshine.ui.view.conversation", { conversation = conv })
    TO_VIEW[conv] = screen.add_view(view)
    screen.print("new conv, title = $1, nick = $2, name = $3", conv:get_title(), conv:get_nick(), conv:get_name())
    screen.render()
end

function H.conversation_destroy(conv)
    assert(TO_VIEW[conv], "conversation exists")
    local view    = screen.detach_view( TO_VIEW[conv] )
    TO_VIEW[conv] = nil
    screen.render()
    log.debug("destroyed conv for %s", view:get_name())
end

function H.conversation_write_im(conv, name,  message, flags, mtime)
    assert(TO_VIEW[conv], "conversation exists")
    local view = screen.get_view(TO_VIEW[conv])
    if name then
        view:print_message({
            level = 3,
            name  = 'private',
            args  = { mtime, name, message },
        })
    else
        -- message from self?
        view:print_message({
            level = 0,
            name  = "private_sent",
            args  = { mtime, conv:get_nick(), message },
        })
    end

    screen.render()
end

function H.conversation_write_chat(conv, name, message, flags, mtime)
    assert(TO_VIEW[conv], "conversation exists")
    local view = screen.get_view(TO_VIEW[conv])
    view:print_message({
        level = 2,
        name  = 'public',
        args  = { mtime, name, message },
    })
    screen.render()
end

function H.conversation_write(conv, name, alias, message, flags, mtime)
    assert(TO_VIEW[conv], "conversation exists")
    local view = screen.get_view(TO_VIEW[conv])
    view:print_message({
        level = 1,
        name  = 'chat',
        args  = { mtime, name, message },
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

function H.conversation_chat_add_users(conv, users, new_arrivals)
    assert(TO_VIEW[conv], "conversation exists")
    local view = screen.get_view(TO_VIEW[conv])

    if new_arrivals then
        log.debug("users are new")
    else
        view:print_userlist(users)
    end
    screen.render()
end

function M.init()
    term.init()
    purple_core.init(H)
end

return M
