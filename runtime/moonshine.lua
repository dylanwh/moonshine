require "moonshine.prelude"
require "moonshine.ui"
local shell  = require "moonshine.shell"
--local irc = require "moonshine.irc"

--local signal = require "moonshine.signal"

--[[shell.define {
	name = "say",
	spec = {"window|win|w=n"},
	action = function(opt, line)
	end,
}]]

--shell.require("window")

local ml = require "moonshine.loop"
loop = ml:new()


loop:run()
