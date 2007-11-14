require "object"
require "window"

Screen = Object:new()
Screen.__type = 'Screen'

define_color("blue", "brightblue", "default")
define_color("white", "white", "default")

function Screen:init()
	local window = Window:new { name = "status" }
	self.windows = { }
	self.window  = nil
	self.entry   = Entry:new()
	self:add(window)
end

function Screen:print(fmt, ...)
	self.window:print(fmt, ...)
	self:render()
end

function Screen:debug(fmt, ...)
	self.window:print("%{blue}-%{white}!%{blue}- %{default}%|"..fmt, ...)
	self:render()
end

function Screen:add(win)
	table.insert(self.windows, win)
	if self.window == nil then
		self.window = win
	end
	win.pos = #self.windows
	return win.pos
end

function Screen:view(x)
	if self.windows[x] then
		self.window = self.windows[x]
		self:render()
		return true
	else
		return false, "invalid window index: " .. tostring(x)
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

