require "moonshine.prelude"
require "moonshine.ui"
local shell  = require "moonshine.shell"
local event  = require "moonshine.event"
local screen = require "moonshine.ui.screen"
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

screen:__init()

loop:run()
