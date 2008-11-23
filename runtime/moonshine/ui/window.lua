local Window = {}

function Window.new(class, name)
	local buffer    = require "moonshine.ui.buffer"
	local statusbar = require "moonshine.ui.statusbar"
	local self      = {}

	self.name     = name
	self.topic    = statusbar:new("")
	self.buffer   = buffer:new(1014)
	self.activity = 0
	self.buffer_dirty = true

	setmetatable(self, { __index = Window })
	self:set_topic("Moonshine - A Haver Client")

	return self
end

function Window:print(fmt, ...)
	self:actprint(1, fmt, ...)
end

function Window:actprint(activity, fmt, ...)
	local term = require "moonshine.ui.term"
	local s    = term.format(os.date("%H:%M ")..tostring(fmt), { ... })

	self.activity = math.max(self.activity, activity)
	self.buffer:print(s)
	self.buffer_dirty = true
end

function Window:render(top, bottom)
	local term       = require "moonshine.ui.term"
	local rows, cols = term.dimensions()

	self.topic:render(top)

	if self.buffer_dirty then
		self.buffer:render(top + 1, bottom)
	end
	self.buffer_dirty = false
end

function Window:scroll(x)
	self.buffer:scroll(x)
	self.buffer_dirty = true
end

function Window:set_topic(t)
	local term = require "moonshine.ui.term"

	self.topic_text = t
	self.topic:set(term.format("%{topic}%1", { t }))
end

function Window:activate()
	self.buffer_dirty = true
end

return Window
