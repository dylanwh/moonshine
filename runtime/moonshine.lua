require "moonshine.prelude"
local loop          = require "moonshine.loop"
local core          = require "purple.core"
local plugins       = require "purple.plugins"
local conversations = require "purple.conversations"

assert(debug.getregistry().BACKREF)

core.init({ui_init = function ()
    conversations.init({
        write_conv = function(conv, name, alias, message, flags, mtime)
            print(conv, name, alias, message, flags, mtime)
        end
    })
end })

local protocols = plugins:get_protocols()

for i, protocol in ipairs(protocols) do
    local info = protocol:get_info()
    print(info.id, info.name)
end

name = "dylan[moonshine]@irc.perl.org"
local account = new("purple.account", name, "prpl-irc")

account:set("port", 6667)
-- account:set_password("")
account:set_enabled(true)

loop:run()

--[===[
local term   = require "moonshine.ui.term"
term.init()

local log    = require "moonshine.log"
local loop   = require "moonshine.loop"
local keymap = new "moonshine.keymap"
local entry  = new "moonshine.ui.entry"
local buffer = new "moonshine.ui.buffer"
local topic  = new "moonshine.ui.label"
local Format = new "moonshine.ui.format"
local purple_core = require "purple.core"
local purple_conversations = require "purple.conversations"

Format:define_style("topic", "white", "blue")
Format:define("topic", "$(style topic)[$1]")
topic:set(Format:apply('topic', "welcome to moonshine!"))

local r, c = term.dimensions()
topic:render(0)
buffer:render(1, r - 2)
entry:render("foo: ")
term.refresh()

function on_input(key)
    if key == 'x' then
        loop.quit()
    elseif key == 'p' then
        purple_core.init({
            ui_init = function()
                purple_conversations.init({
                    write_conv = function (conv, name, alias, message, flags, mtime)
                        buffer:print(message)
                        topic:render(0)
                        buffer:render(1, r - 2)
                        entry:render("[status] ")
                        term.refresh()
                    end,
                })
                topic:set(Format:apply('topic', "welcome to purple moonshine!"))
                topic:render(0)
                entry:render("[status] ")
                term.refresh()
            end
        })
    else
        entry:keypress(key)
        topic:render(0)
        buffer:render(1, r - 2)
        entry:render("[status] ")
        term.refresh()
    end
end

loop.run()

term.reset()
]===]
