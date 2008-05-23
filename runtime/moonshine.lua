require "moonshine.prelude"

term = require "moonshine.ui.term"
Buffer = require "moonshine.ui.buffer"
Entry  = require "moonshine.ui.entry"
Statusbar = require "moonshine.ui.statusbar"

function main()
	term.setup {
		input  = hook "input",
		resize = hook "resize",
	}
	
	atexit(term.reset)
end
