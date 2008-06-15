local term      = require "moonshine.ui.term"
local buffer    = require "moonshine.ui.buffer"
local statusbar = require "moonshine.ui.statusbar"
local object    = require "moonshine.object"

local Window = object:clone ()

function Window:init(...)
	print "init window..."
	object.init(self, ...)

	self.topic  = statusbar:new("")
	self.buffer = buffer:new(1014)
	self.activity = 0
	self:set_topic("Moonshine - A Haver Client")
	self.bufferdirty = true
end

function Window:print(fmt, ...)
	self:actprint(1, fmt, ...)
end

function Window:actprint(activity, fmt, ...)
	self.activity = math.max(self.activity, activity)
	local s = term.format(os.date("%H:%M ")..tostring(fmt), arg)
	self.buffer:print(s)
	self.bufferdirty = true
end

function Window:render(toprow, bottomrow)
	local rows, cols = term.dimensions()
	self.topic:render(toprow)
	if self.bufferdirty then
		self.buffer:render(toprow + 1, bottomrow)
	end
	self.bufferdirty = false
end

function Window:scroll(x)
	self.buffer:scroll(x)
	self.bufferdirty = true
end

function Window:set_topic(t)
	self.topic_text = t
	self.topic:set(term.format("%{topic}%1", { t }))
end

function Window:activate()
	self.bufferdirty = true
end

return Window
