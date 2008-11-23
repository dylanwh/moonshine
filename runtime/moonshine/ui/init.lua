local signal = require "moonshine.signal"
local term   = require "moonshine.ui.term"

term.setup {
	input  = function(key) signal.emit("input", key) end,
	resize = function() signal.emit("render") end,
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


