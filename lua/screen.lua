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
	self:view(
		self:add(window)
	)
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
		self.entry:set_prompt("[" .. (self.window.name or '???' ) .. "] ")
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
	self.entry:move_to(0)
	self:render()
end

function Screen:move_end()
	self.entry:move_to(-1)
	self:render()
end

function Screen:backspace()
	self.entry:erase(-1) 
	self:render()
end

function Screen:word_delete()
	local wlback, wlfwd = screen.entry:wordlen()
	self:debug("back=%1 fwd=%2", wlback, wlfwd)
	screen.entry:erase(-wlback)
	self:render()
end

function Screen:word_left()
	local wlback, wlfwd = screen.entry:wordlen()
	self:debug("back=%1 fwd=%2", wlback, wlfwd)
	screen.entry:move(-wlback)
	self:render()
end

function Screen:word_right()
	local wlback, wlfwd = screen.entry:wordlen()
	self:debug("back=%1 fwd=%2", wlback, wlfwd)
	screen.entry:move(wlfwd)
	self:render()
end

function Screen:send_line(f)
	local line = screen.entry:get()
	if #line > 0 then
		screen.entry:clear()
		f(line)
		self:render()
	end
end

