local purple_core          = require "purple.core"
local purple_conversations = require "purple.conversations"

local log    = require "moonshine.log"
local term   = require "moonshine.ui.term"
local shell  = require "moonshine.shell"
local keymap = new "moonshine.keymap"

local format = require "moonshine.ui.format"
local screen = require "moonshine.ui.screen"

local CONV_VIEW = { }

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
    term.init()
    format.init()
    screen.init()

    keymap:bind('{kbs}',   function () screen.entry_erase(-1)          end)
    keymap:bind('{kcub1}', function () screen.entry_move(-1)           end)
    keymap:bind('{kcuf1}', function () screen.entry_move(1)            end)
    keymap:bind('{kcuu1}', function () screen.history_backward()       end)
    keymap:bind('{kcud1}', function () screen.history_forward()        end)

    local accept = shell.accept_line
    keymap:bind('{kent}',  function () screen.entry_submit(accept) end)

    -- why does urxvt not sent kent?
    keymap:bind('^M',  function () screen.entry_submit(accept) end)

    screen.render()
end

function H.create_conversation(conv)
    assert(CONV_VIEW[conv] == nil, "conversation does not exist")
    screen.print("hello, world")
    local view = new("moonshine.ui.view", { name = conv:get_name(), conv = conv })
    CONV_VIEW[conv] = screen.add_view(view)
end

function H.destroy_conversation(conv)
    assert(CONV_VIEW[conv], "conversation exists")
    screen.del_view( CONV_VIEW[conv] )
end

function H.write_im(conv, name,  message, flags, mtime)
    assert(CONV_VIEW[conv], "conversation exists")
    screen.focus_view(CONV_VIEW[conv])
end

function H.write_chat(conv, name, message, flags, mtime)
    assert(CONV_VIEW[conv], "conversation exists")
    screen.focus_view(CONV_VIEW[conv])
end

function H.present(conv)
    assert(CONV_VIEW[conv], "conversation exists")
    screen.focus_view(CONV_VIEW[conv])
end

function H.has_focus(conv)
    assert(CONV_VIEW[conv], "conversation exists")
    return screen.is_focused(CONV_VIEW[conv])
end

local M = {}

function M.init()
    purple_core.init(H)
end

return M
