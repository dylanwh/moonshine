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
local screen = require "moonshine.ui.screen.main"
local shell  = require "moonshine.shell"

--{{{ keymap stuff.
keymap.register('redraw',        screen:callback "render")
keymap.register('submit',        screen:callback "submit")
keymap.register('backspace',     screen:callback "backspace")
keymap.register('page_up',       screen:callback "scroll_up")
keymap.register('page_down',     screen:callback "scroll_down")
keymap.register('left',          screen:callback "move_left")
keymap.register('right',         screen:callback "move_right")
keymap.register('home',          screen:callback "move_home")
keymap.register('end',           screen:callback "move_end")
keymap.register('up',            screen:callback "history_backward")
keymap.register('down',          screen:callback "history_forward")
keymap.register('delete_word',   screen:callback "word_delete")
keymap.register('left_by_word',  screen:callback "word_left")
keymap.register('right_by_word', screen:callback "word_right")
keymap.register('command',       shell.eval)

keymap.bind("^[[A",    "up")
keymap.bind("^[[B",    "down")
keymap.bind("^[[C",    "right")
keymap.bind("^[[D",    "left")
keymap.bind("^[[H",    "home")
keymap.bind("^[[F",    "end")

keymap.bind("^A",      "home")
keymap.bind("^[[7~",   "home")
keymap.bind("^[[1~",   "home")
keymap.bind("^[[8~",   "end")
keymap.bind("^[[4~",   "end")
keymap.bind("^[[5~",   "page_up")
keymap.bind("^[[6~",   "page_down")
keymap.bind("^[[1;5D", "left_by_word")
keymap.bind("^[[1;5C", "right_by_word")
keymap.bind("^W",      "delete_word")
keymap.bind("^?",      "backspace")
keymap.bind("^H",      "backspace")
keymap.bind("^M",      "submit")
keymap.bind("^C",      "command", "/quit")
keymap.bind("^L",      "redraw")
--}}}

local log = require "moonshine.log"

add_hook("unknown hook", function (name, args)
	log('warning', "unknown hook: %s (%s)", name, join("|", args or {}))
end)

add_hook("unknown command", function(name)
	log("warning", "unknown command: /%s", name)
end)

add_hook("shell error", function(err)
	log("critical", "error in shell command: %s", err)
end)

add_hook('unknown protocol command', function(tag, info)
	log('warning', "unknown protocol command: %s (%s)", info.name, join(",", info.args or {}))
end)

add_hook("log", function (domain, level, message)
	local f = io.open("moonshine.log", "a")
	f:write(string.format("[%s] %s\n", level, message))
	f:close()
	screen:debug("[%1] %2", level, message)
end)

add_hook('private message', function(tag, user, kind, text)
	local window = screen:window_for_user(tag, user)
	window:print("<%1> %|%2", user, text)
	screen:render()
end)

add_hook('public message', function(tag, room, user, kind, text)
	local window = screen:window_for_room(tag, room)
	window:print("[%1] <%2> %|%3", room, user, text)
	screen:render()
end)

add_hook('topic', function(tag, room, text)
	local window = screen:window_for_room(tag, room)
	window:set_topic(text)
	screen:render()
end)

add_hook("input",    keymap.process)
add_hook("keypress", screen:callback "keypress")
add_hook("resize",   screen:callback "resize")

screen:render()
