require "object"
require "ui.window"

Screen = Object:clone { __type = 'Screen' }

define_color("blue", "brightblue", "default")
define_color("white", "white", "default")

function Screen:init()
	local window = Window:new { name = "status" }
	self.windows = { }
	self.window  = nil
	self.entry   = Entry:new()
	self.status  = Statusbar.new()

	self.statusbits = {
		self.status_time,
		self.status_current_window,
		self.status_activity
	}
	self.windows = { }
	self.window  = nil
	self:view(
		self:add(window)
	)
	local sbartext = Buffer.format("%{topic}Status bar goes here", {})
	self.status:set(sbartext)
end

function Screen:status_time()
	return os.date("%H:%M")
end

function Screen:status_current_window()
	if self.window then
		return (self.window.pos .. ":" .. (self.window.name or '???'))
	else
		return nil
	end
end

function Screen:status_activity()
	-- TODO
	return Buffer.format("FakeAct: %{statusboring}1,%{statusnormal}2%{statusboring},%{statusimportant}3", {});
end

function Screen:updatestatus()
	local statusbuf = ""
	for i, bit in ipairs(self.statusbits) do
		local result = bit(self)
		if result then
			statusbuf = statusbuf .. Buffer.format("%{statusbracket} [%{statustext}%1%{statusbracket}]", {result})
		end
	end
	self.status:set(statusbuf)
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
	win:set_topic("Moonshine - " .. (win.name or '???'));
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
	local rows, cols = term_dimensions()
	self.window:render(0, rows - 3)
	self:updatestatus()
	self.status:render(rows - 2)
	self.entry:render()
	refresh()
	status("Moonshine - " .. (self.window.name or '???'));
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
	screen.entry:erase(-wlback)
	self:render()
end

function Screen:word_left()
	local wlback, wlfwd = screen.entry:wordlen()
	screen.entry:move(-wlback)
	self:render()
end

function Screen:word_right()
	local wlback, wlfwd = screen.entry:wordlen()
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

