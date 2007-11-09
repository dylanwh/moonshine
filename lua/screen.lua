require "object"
require "window"

Screen = Object:new()

function Screen:init()
	self.focus   = 1
	self.windows = { Window:new() }
	self.window  = self.windows[self.focus]
	self.window.topic:set("status window")

	self.entry   = Entry.new();
end

function Screen:print(fmt, ...)
	self.window:print(fmt, ...)
	self:render()
end

function Screen:debug(fmt, ...)
	self.window:print("%{debug}[debug]%{default} %|"..fmt, ...)
	self:render()
end

function Screen:view(id)
	if self.windows[id] then
		self.focus  = id
		self.window = self.windows[self.focus]
		self:render()
		return true
	else
		return false, "invalid window number"
	end
end

function Screen:render()
	self.window:render()
	self.entry:render()
	refresh()
end

function Screen:keypress(key)
	self.entry:keypress(key)
	self:render()
end

function Screen:scroll_up()
	self.window:scroll(5)
	self:render()
end

function Screen:scroll_down()
	self.window:scroll(-5)
	self:render()
end

function Screen:move_left() 
	self.entry:move(-1)
	self:render()
end

function Screen:move_right()
	self.entry:move(1)
	self:render()
end

function Screen:move_home()
	self.entry:move_to(-1)
	self:render()
end

function Screen:move_end()
	entry:move_to(1)
	render()
end

function Screen:backspace()
	self.entry:erase(-1) 
	self:render()
end

function Screen:send_line(f)
	local line = screen.entry:get()
	screen.entry:clear()
	f(line)
	self:render()
end

