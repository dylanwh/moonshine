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

function Window:render(toprow, bottomrow)
	local rows, cols = term_dimensions()
	self.topic:render(toprow)
	self.buffer:render(toprow + 1, bottomrow)
end

function Window:scroll(x)
	self.buffer:scroll(x)
end

function Window:set_topic(t)
	self.topic:set(Buffer.format("%{topic}%1", { t }))
end
