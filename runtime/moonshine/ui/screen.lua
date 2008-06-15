local term      = require "moonshine.ui.term"
local Entry     = require "moonshine.ui.entry"
local Buffer    = require "moonshine.ui.buffer"
local Statusbar = require "moonshine.ui.statusbar"
local Window    = require "moonshine.ui.window"

local object    = require "moonshine.object"
local Screen    = object:clone ()

term.defcolor("blue", "brightblue", "default")
term.defcolor("white", "white", "default")
term.defcolor("topic", "lightgray", "blue")
term.defcolor("statusbracket", "cyan", "blue")
term.defcolor("statustext", "lightgray", "blue")
term.defcolor("statusboring", "cyan", "blue")
term.defcolor("statusnormal", "white", "blue")
term.defcolor("statusimportant", "brightmagenta", "blue")
term.defcolor("self", "white", "default")


function Screen:init(...)
	print "init screen..."
	object.init(self, ...)

	self.entry   = Entry:new()
	self.status  = Statusbar:new()
	self.history = Buffer:new()
	self.sb_at_end = true

	self.statusbits = {
		self.status_time,
		self.status_current_window,
		self.status_activity
	}
	self.windows = { }
	self.window  = nil

	local window = Window:clone { name = "status" }
	self:view( self:add(window) )

	local sbartext = term.format("%{topic}Status bar goes here", {})
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
	return term.format(actString, {})
end

function Screen:updatestatus()
	local statusbuf = ""
	for i, bit in ipairs(self.statusbits) do
		local result = bit(self)
		if result then
			statusbuf = statusbuf .. term.format("%{statusbracket} [%{statustext}%1%{statusbracket}]", {result})
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

function Screen:remove(win)
	assert(win.pos ~= nil, "window has position")
	if self.window == win then
		self.window = self.windows[win.pos - 1]
		self.window:set_topic("Moonshine - " .. (win.name or '???'));
		self.entry:set_prompt("[" .. (self.window.name or '???' ) .. "] ")
	end
	table.remove(Screen.windows, win.pos)
	for i, w in ipairs(self.windows) do
		w.pos = i
	end
end

function Screen:view(x)
	print ("view", x)
	if self.windows[x] then
		self.window = self.windows[x]
		self.entry:set_prompt("[" .. (self.window.name or '???' ) .. "] ")
		self.window:activate()
		self:render()
		return true
	else
		return false, "invalid window index: " .. tostring(x)
	end
end

function Screen:resize()
	print ("self", self)
	self:render()
end

function Screen:render()
	local rows, cols = term.dimensions()
	self.window:render(0, rows - 3)
	self:updatestatus()
	self.status:render(rows - 2)
	self.entry:render()
	term.refresh()
	term.status(self.window.topic_text or 'O.o')
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
	local wlback, wlfwd = self.entry:wordlen()
	Screen.entry:erase(-wlback)
	self:render()
end

function Screen:word_left()
	local wlback, wlfwd = self.entry:wordlen()
	self.entry:move(-wlback)
	self:render()
end

function Screen:word_right()
	local wlback, wlfwd = self.entry:wordlen()
	self.entry:move(wlfwd)
	self:render()
end

function Screen:send_line(f)
	local line = self.entry:get()
	if #line > 0 then
		self:history_save()
		self.history:scroll_to(0)
		self.sb_at_end = true
		self.entry:clear()
		f(line)
		self:render()
	end
end

function Screen:history_save()
	if not self.entry:is_dirty() or self.entry:get() == "" then
		return
	end
	-- Suppress repeated lines from going into history
	if self.history:at_end() and self.entry:get() == self.history:get_current() then
		return
	end
	self.history:print(self.entry:get())
end

function Screen:history_backward()
	self:history_save()
	-- Are we just starting to scroll back?
	if not self.sb_at_end then
		self.history:scroll(1)
	end
	self.sb_at_end = false
	local v = self.history:get_current()
	if v then
		self.entry:set(v)
	end
	self:render()
end

function Screen:history_forward()
	self:history_save()
	if self.history:at_end() then
		self.entry:clear()
		self.sb_at_end = true
	else
		self.history:scroll(-1)
		local v = self.history:get_current()
		if v then
			self.entry:set(v)
		end
	end
	self:render()
end

Screen:init()

return Screen
