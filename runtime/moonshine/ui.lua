local signal = require "moonshine.signal"
local keymap = require "moonshine.ui.keymap"
require "moonshine.ui.init"
require "moonshine.ui.screen"

keymap.bind("^[[A",     "entry history prev")
keymap.bind("^[[B",     "entry history next")
keymap.bind("^[[C",     "entry move right")
keymap.bind("^[[D",     "entry move left")
keymap.bind("^[[H",     "entry move home")
keymap.bind("^[[F",     "entry move end")

keymap.bind("^A",       "entry move home")
keymap.bind("^[[7~",    "entry move home")
keymap.bind("^[[8~",    "entry move end")
keymap.bind("^[[5~",    "buffer scroll up")
keymap.bind("^[[6~",    "buffer scroll down")
keymap.bind("^[[1;5D",  "entry move left by word")
keymap.bind("^[[1;5C",  "entry move right by word")
keymap.bind("^W",       "entry delete word")
keymap.bind("^?",       "entry backspace")
keymap.bind("^H",       "entry backspace")
keymap.bind("^M",       "entry submit")
keymap.bind("^C",       "command quit")
keymap.bind("^L",       "screen refresh")

for i = 1, 9 do
	keymap.bind("^[" .. i, "window goto", i)
end

signal.add("exit", quit)
