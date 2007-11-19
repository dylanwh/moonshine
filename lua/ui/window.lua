require "object"

Window = Object:clone { __type = 'Window' }

function Window:init()
	self.topic  = Statusbar:new("")
	self.buffer = Buffer:new()
	self:set_topic("Moonshine - A Haver Client")
end

function Window:print(fmt, ...)
	fmt = os.date("%H:%M ")..fmt
	local s = Buffer.format(tostring(fmt), arg)
	self.buffer:print(s)
end

function Window:render(fmt, ...)
	local rows, cols = term_dimensions()
	self.topic:render(0)
	self.buffer:render(1, rows - 2)
end

function Window:scroll(x)
	self.buffer:scroll(x)
end

function Window:set_topic(t)
	self.topic:set(Buffer.format("%{topic}%1", { t }))
end
