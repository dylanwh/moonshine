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
	self.scrollback = Buffer:new()
	self.sb_at_end = true

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
	local actvals = { }
	if self.window then
		self.window.activity = 0
	end
	for i, window in pairs(self.windows) do
		if (window.activity > 0) then
			local code = "important"
			if window.activity == 1 then
				code = "boring"
			elseif window.activity == 2 then
				code = "normal"
			end
			table.insert(actvals, "%{status"..code.."}"..i)
		end
	end
	if table.getn(actvals) == 0 then
		return nil
	end

	local actString = "Act: "..join("%{statusboring},", actvals)
	return Buffer.format(actString, {})
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
		self:scrollback_save()
		self.scrollback:scroll_to(0)
		self.sb_at_end = true
		screen.entry:clear()
		f(line)
		self:render()
	end
end

function Screen:scrollback_save()
	if not self.entry:is_dirty() then
		return
	end
	self:debug("Saving scrollback")
	self.scrollback:print(self.entry:get())
end

function Screen:entry_up()
	self:scrollback_save()
	-- Are we just starting to scroll back?
	if not self.sb_at_end then
		self.scrollback:scroll(1)
	end
	self.sb_at_end = false
	local v = self.scrollback:get_current()
	if v then
		self:debug("Set entry to (%1)", v)
		self.entry:set(v)
	end
	self:render()
end

function Screen:entry_down()
	self:scrollback_save()
	if self.scrollback:at_end() then
		self.entry:clear()
		self.sb_at_end = true
	else
		self.scrollback:scroll(-1)
		local v = self.scrollback:get_current()
		if v then
			self:debug("Set entry to (%1)", v)
			self.entry:set(v)
		end
	end
	self:render()
end
