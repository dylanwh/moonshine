local command = require "moonshine.command"
local term    = require "moonshine.ui.term"
local keymap  = require "moonshine.ui.keymap"
local screen  = require "moonshine.ui.screen"

local bind   = keymap:callback "bind"

function main(argv)
	term.setup {
		input  = keymap:callback "process",
		resize = screen:callback "resize",
	}

	bind("",        screen:callback "keypress")
	bind("^[[A",    screen:callback "history_backward")
	bind("^[[B",    screen:callback "history_forward")
	bind("^[[H",    screen:callback "move_home")
	bind("^[[F",    screen:callback "move_end")
	bind("^[7~",    screen:callback "move_home")
	bind("^[8~",    screen:callback "move_end")
	bind("^[[C",    screen:callback "move_right")
	bind("^[[D",    screen:callback "move_left")
	bind("^[[5~",   screen:callback "scroll_up")
	bind("^[[6~",   screen:callback "scroll_down")
	bind("^[[1;5D", screen:callback "word_left")
	bind("^[[1;5C", screen:callback "word_right")
	bind("^W",      screen:callback "word_delete")
	bind("^?",      screen:callback "backspace")
	bind("^H",      screen:callback "backspace")
	bind("^C",      screen:callback "quit")
	bind("^L",      screen:callback "redraw")
	bind("^M",      screen:callback("send_line", command:callback "eval") )

	for i = 1, 9 do
		bind("^[" .. i, "view " .. i)
	end
end
