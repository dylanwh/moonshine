require "moonshine.prelude"

local term   = require "moonshine.ui.term"
term.init()

local log    = require "moonshine.log"
local loop   = require "moonshine.loop"
local keymap = new "moonshine.keymap"
local entry  = new "moonshine.ui.entry"
local buffer = new "moonshine.ui.buffer"
local topic  = new "moonshine.ui.label"
local Format = new "moonshine.ui.format"


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
    else
        entry:keypress(key)
    end
    topic:render(0)
    buffer:render(1, r - 2)
    entry:render("[status] ")
    term.refresh()
end

loop.run()

term.reset()
