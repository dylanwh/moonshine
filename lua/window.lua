require "object"

Window = Object:new()

function Window:init()
	self.topic  = Topic.new("Moonshine - A Haver Client")
	self.buffer = Buffer.new()
end

function Window:print(fmt, ...)
	fmt = os.date("%H:%M ")..fmt
	local s = Buffer.format(tostring(fmt), arg)
	self.buffer:print(s)
end

function Window:render(fmt, ...)
	self.topic:render()
	self.buffer:render()
end

function Window:scroll(x)
	self.buffer:scroll(x)
end

function Window:set_topic(t)
	self.topic:set(t)
end
