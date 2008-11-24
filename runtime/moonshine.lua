--local shell  = require "moonshine.shell" 
require "moonshine.prelude"
require "moonshine.ui"

local signal = require "moonshine.signal"
local shell  = require "moonshine.shell"

shell.define {
	name = "say",
	spec = {"window|win|w=n"},
	action = function(opt, line)
		print(line)
	end,
}

shell.require("window")
shell.require("connect")

signal.emit("startup")
main_loop()
signal.emit("shutdown")
