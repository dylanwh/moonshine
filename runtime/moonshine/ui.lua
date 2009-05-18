local term   = require "moonshine.ui.term"
term.defcolor("blue", "brightblue", "default")
term.defcolor("white", "white", "default")
term.defcolor("topic", "lightgray", "blue")
term.defcolor("statusbracket", "cyan", "blue")
term.defcolor("statustext", "lightgray", "blue")
term.defcolor("statusboring", "cyan", "blue")
term.defcolor("statusnormal", "white", "blue")
term.defcolor("statusimportant", "brightmagenta", "blue")
term.defcolor("self", "white", "default")

local keymap = require "moonshine.ui.keymap"
local loop   = require "moonshine.loop"
local screen = require "moonshine.ui.screen.main"

keymap.define("screen redraw",      screen:callback "render")
keymap.define("window goto",        screen:callback "goto")
keymap.define("entry submit",       screen:callback "submit")
keymap.define("entry backspace",    screen:callback "backspace")
keymap.define("buffer scroll up",   screen:callback "scroll_up")
keymap.define("buffer scroll down", screen:callback "scroll_down")
keymap.define("entry move left",    screen:callback "move_left")
keymap.define("entry move right",   screen:callback "move_right")
keymap.define("entry move home",    screen:callback "move_home")
keymap.define("entry move end",     screen:callback "move_end")
keymap.define("entry history prev", screen:callback "history_backward")
keymap.define("entry history next", screen:callback "history_forward")
keymap.define("entry delete word",  screen:callback "word_delete")
keymap.define("entry move left by word",  screen:callback "word_left")
keymap.define("entry move right by word", screen:callback "word_right")

keymap.bind("^[[A",    "entry history prev")
keymap.bind("^[[B",    "entry history next")
keymap.bind("^[[C",    "entry move right")
keymap.bind("^[[D",    "entry move left")
keymap.bind("^[[H",    "entry move home")
keymap.bind("^[[F",    "entry move end")

keymap.bind("^A",      "entry move home")
keymap.bind("^[[7~",   "entry move home")
keymap.bind("^[[1~",   "entry move home")
keymap.bind("^[[8~",   "entry move end")
keymap.bind("^[[4~",   "entry move end")
keymap.bind("^[[5~",   "buffer scroll up")
keymap.bind("^[[6~",   "buffer scroll down")
keymap.bind("^[[1;5D", "entry move left by word")
keymap.bind("^[[1;5C", "entry move right by word")
keymap.bind("^W",      "entry delete word")
keymap.bind("^?",      "entry backspace")
keymap.bind("^H",      "entry backspace")
keymap.bind("^M",      "entry submit")
keymap.bind("^C",      "quit")
keymap.bind("^L",      "screen redraw")

for i = 1, 9 do
	keymap.bind("^[" .. i, "window goto", i)
end


kb_quit = loop.quit
on_input    = keymap.process
on_keypress = screen:callback "keypress"
on_resize   = screen:callback "resize"

screen:render()
