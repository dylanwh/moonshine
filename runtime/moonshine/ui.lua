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

local keymap = require "moonshine.keymap"
local loop   = require "moonshine.loop"
local screen = require "moonshine.ui.screen.main"

--{{{ keymap stuff.
kb_screen_redraw            = screen:callback "render"
kb_window_goto              = screen:callback "goto"
kb_entry_submit             = screen:callback "submit"
kb_entry_backspace          = screen:callback "backspace"
kb_buffer_scroll_up         = screen:callback "scroll_up"
kb_buffer_scroll_down       = screen:callback "scroll_down"
kb_entry_move_left          = screen:callback "move_left"
kb_entry_move_right         = screen:callback "move_right"
kb_entry_move_home          = screen:callback "move_home"
kb_entry_move_end           = screen:callback "move_end"
kb_entry_history_prev       = screen:callback "history_backward"
kb_entry_history_next       = screen:callback "history_forward"
kb_entry_delete_word        = screen:callback "word_delete"
kb_entry_move_left_by_word  = screen:callback "word_left"
kb_entry_move_right_by_word = screen:callback "word_right"
kb_quit                     = loop.quit

keymap.bind("^[[A",    "entry_history_prev")
keymap.bind("^[[B",    "entry_history_next")
keymap.bind("^[[C",    "entry_move_right")
keymap.bind("^[[D",    "entry_move_left")
keymap.bind("^[[H",    "entry_move_home")
keymap.bind("^[[F",    "entry_move_end")

keymap.bind("^A",      "entry_move_home")
keymap.bind("^[[7~",   "entry_move_home")
keymap.bind("^[[1~",   "entry_move_home")
keymap.bind("^[[8~",   "entry_move_end")
keymap.bind("^[[4~",   "entry_move_end")
keymap.bind("^[[5~",   "buffer_scroll_up")
keymap.bind("^[[6~",   "buffer_scroll_down")
keymap.bind("^[[1;5D", "entry_move_left_by_word")
keymap.bind("^[[1;5C", "entry_move_right_by_word")
keymap.bind("^W",      "entry_delete_word")
keymap.bind("^?",      "entry_backspace")
keymap.bind("^H",      "entry_backspace")
keymap.bind("^M",      "entry_submit")
keymap.bind("^C",      "quit")
keymap.bind("^L",      "screen_redraw")

for i = 1, 9 do
	keymap.bind("^[" .. i, "window_goto", i)
end
--}}}

function on_unknown_hook(name)
	screen:debug("unknown hook: " .. name)
end

function on_command_error(err)
	screen:debug("COMMAND ERROR: " .. err)
end

function on_log(domain, level, message)
	local f = io.open("moonshine.log", "a")
	f:write(string.format("[%s] %s\n", level, message))
	f:close()
	screen:debug("[%1] %2", level, message)
end

function on_unknown_command(name)
	screen:debug("command " .. name .. " not found")
end

on_input    = keymap.process
on_keypress = screen:callback "keypress"
on_resize   = screen:callback "resize"

screen:render()
