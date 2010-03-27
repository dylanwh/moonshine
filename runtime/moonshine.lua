require "moonshine.prelude"

local term   = require "moonshine.ui.term"
term.init()

local log    = require "moonshine.log"
local loop   = require "moonshine.loop"
local keymap = new "moonshine.keymap"
local entry  = new "moonshine.ui.entry"
local buffer = new "moonshine.ui.buffer"
local label  = new "moonshine.ui.label"
local Format = new "moonshine.ui.format"


Format:define_color("aqua", 0, 454, 1000)
Format:define_style("topic", "color12", "black")
Format:define_style("debug", "blue", "black")

Format:define("topic", "${style topic}$1")
Format:define("debug", "[debug]$| ${style debug}$1")
Format:define("info",  "${debug ${concat we have $1 colors and $2 styles}}")

buffer:print(Format:apply('info', term.colors(), term.styles()))

label:set(Format:apply("topic", "TACOS!"))

local r, c = term.dimensions()
label:render(0)
buffer:render(1, r - 2)
entry:render("foo: ")
term.refresh()

function on_input(key)
    if key == 'x' then
        loop.quit()
    else
        entry:keypress(key)
    end
    label:render(0)
    buffer:render(1, r - 2)
    entry:render("foo: ")
    term.refresh()
end

loop.run()

term.reset()
