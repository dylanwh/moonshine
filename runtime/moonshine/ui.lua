local term   = require "moonshine.ui.term"
local keymap = require "moonshine.ui.keymap"
local screen = require "moonshine.ui.screen.main"

term.setup {
	input  = keymap.keypress,
	resize = screen:callback "render",
}

term.defcolor("blue", "brightblue", "default")
term.defcolor("white", "white", "default")
term.defcolor("topic", "lightgray", "blue")
term.defcolor("statusbracket", "cyan", "blue")
term.defcolor("statustext", "lightgray", "blue")
term.defcolor("statusboring", "cyan", "blue")
term.defcolor("statusnormal", "white", "blue")
term.defcolor("statusimportant", "brightmagenta", "blue")
term.defcolor("self", "white", "default")

keymap.bind("^[[A",     "entry history prev")
keymap.bind("^[[B",     "entry history next")
keymap.bind("^[[C",     "entry move right")
keymap.bind("^[[D",     "entry move left")
keymap.bind("^[[H",     "entry move home")
keymap.bind("^[[F",     "entry move end")

keymap.bind("^A",       "entry move home")
keymap.bind("^[[7~",    "entry move home")
keymap.bind("^[[1~",    "entry move home")
keymap.bind("^[[8~",    "entry move end")
keymap.bind("^[[4~",    "entry move end")
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

