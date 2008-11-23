local signal = require "moonshine.signal"
local keymap = require "moonshine.ui.keymap"
require "moonshine.ui.init"
require "moonshine.ui.screen"

keymap.bind("^[[A",    "key page_up")
keymap.bind("^[[B",    "key page_down")
keymap.bind("^[[C",    "key right")
keymap.bind("^[[D",    "key left")
keymap.bind("^[[H",    "key home")
keymap.bind("^[[F",    "key end")

keymap.bind("^A",       "key home")
keymap.bind("^[[7~",    "key home")
keymap.bind("^[[8~",    "key end")
keymap.bind("^[[5~",   "key up")
keymap.bind("^[[6~",   "key down")
keymap.bind("^[[1;5D", "key word_left")
keymap.bind("^[[1;5C", "key word_right")
keymap.bind("^W",      "key word_delete")
keymap.bind("^?",      "key backspace")
keymap.bind("^H",      "key backspace")
keymap.bind("^M",      "key return")
keymap.bind("^C",      "exit")
keymap.bind("^L",      "render")

for i = 1, 9 do
	keymap.bind("^[" .. i, "view", i)
end

signal.add("exit", os.exit)
