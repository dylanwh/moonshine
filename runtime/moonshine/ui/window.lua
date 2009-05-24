local term      = require "moonshine.ui.term"
local Object    = require "moonshine.object"
local Buffer    = require "moonshine.ui.buffer"
local Statusbar = require "moonshine.ui.statusbar"

local Window    = Object:clone()

function Window:__init()
	assert(self.name, "name slot initialized")

	self._topic    = Statusbar:new("")
	self._buffer   = Buffer:new(1014)
	self._activity = 0

	self:set_topic("Moonshine - A Haver Client")
end

function Window:print(fmt, ...)
	self:actprint(1, fmt, ...)
end

function Window:actprint(activity, fmt, ...)
	local str = term.format(os.date("%H:%M ")..tostring(fmt), { ... })

	self._activity = math.max(self._activity, activity)
	self._buffer:print(str)
end

function Window:render(top, bottom)
	local rows, cols = term.dimensions()

	self._topic:render(top)
	if self._buffer:is_dirty() then
		self._buffer:render(top + 1, bottom)
	end
end

function Window:scroll(x)
	self._buffer:scroll(x)
end

function Window:set_topic(t)
	self._topic:set(term.format("%{topic}%1", { t }))
end

function Window:activate()
	self._buffer:is_dirty(true)
end

function Window:resize()
	self._buffer:is_dirty(true)
end

return Window
