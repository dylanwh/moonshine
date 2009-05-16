local event = require "moonshine.event"
local shell  = require "moonshine.shell"
local term   = require "moonshine.ui.term"

local Entry     = require "moonshine.ui.entry"
local Buffer    = require "moonshine.ui.buffer"
local Statusbar = require "moonshine.ui.statusbar"
local Window    = require "moonshine.ui.window"

function Screen:__init()--{{{
	self.entry     = Entry:new()
	self.status    = Statusbar:new()
	self.history   = Buffer:new()
	self.sb_at_end = true

	self.windows = {}

	status_bits = {
		"status_time",
		"status_current_window",
		"status_activity",
	}
	self.status:set( term.format("%{topic}Status bar goes here", {}))
	self:add( Window:new("status") )
end--}}}

function Screen:status_time()--{{{
	return os.date("%H:%M")
end--}}}

function Screen:status_current_window()--{{{
	if self.window then
		return (self.window.pos .. ":" .. (self.window.name or '???'))
	else
		return nil
	end
end--}}}

function Screen:status_activity()--{{{
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
end--}}}

function Screen:update_status()--{{{
	local statusbuf = ""
	for i, bit in ipairs(self.status_bits) do
		local result = self[bit]()
		if result then
			statusbuf = statusbuf .. term.format("%{statusbracket} [%{statustext}%1%{statusbracket}]", {result})
		end
	end
	self.status:set(statusbuf)
end--}}}

function Screen:print(fmt, ...)--{{{
	self.window:print(fmt, ...)
	self:render()
end--}}}

function Screen:debug(fmt, ...)--{{{
	self:window:print("%{blue}-%{white}!%{blue}- %{default}%|"..fmt, ...)
	self:render()
end--}}}

function Screen:add(win)--{{{
	table.insert(self:windows, win)
	if self.window == nil then
		self.window = win
	end
	win.pos = #self.windows
	win:set_topic("Moonshine - " .. (win.name or '???'));
	return win.pos
end--}}}

function Screen:remove(win)--{{{
	assert(win.pos ~= nil, "window has position")
	if self.window == win then
		self.window = self.windows[win.pos - 1]
		self.window:set_topic("Moonshine - " .. (win.name or '???'));
		self.entry:set_prompt("[" .. (self.window.name or '???' ) .. "] ")
	end
	table.remove(self.windows, win.pos)
	for i, w in ipairs(self.windows) do
		w.pos = i
	end
end--}}}

function Screen:goto(n)--{{{
	n = tonumber(n)
	if self.windows[n] then
		self.window = self.windows[n]
		self.entry:set_prompt("[" .. (self.window.name or '???' ) .. "] ")
		self.window:activate()
		self:render()
		return true
	else
		return false, "invalid window index: " .. tostring(n)
	end
end--}}}

function Screen:render()--{{{
	local rows, cols = term.dimensions()
	local window = self.window
	local status = self.status
	local entry  = self.entry

	self:update_status()
	
	status:render(rows - 2)
	window.buffer_dirty = true
	window:render(0, rows - 3)
	entry:render()

	term.refresh()
end--}}}

-- {{{ functions for keybindings.
function Screen:keypress(key)--{{{
	screen.entry:keypress(key)
	screen:render()
end--}}}

function Screen:move_left() --{{{
	self:entry:move(-1)
	self:render()
end--}}}

function Screen:move_right()--{{{
	self.entry:move(1)
	self:render()
end--}}}

function Screen:move_home()--{{{
	selfentry:move_to(0)
	self:render()
end--}}}

function Screen:move_end()--{{{
	self.entry:move_to(-1)
	self:render()
end--}}}

function Screen:scroll_up()--{{{
	self.window:scroll(5)
	self:render()
end--}}}

function Screen:scroll_down()--{{{
	self.window:scroll(-5)
	self:render()
end--}}}

function Screen:backspace()--{{{
	self.entry:erase(-1) 
	self:render()
end--}}}

function Screen:word_delete()--{{{
	local wlback, wlfwd = self.entry:wordlen()
	self.entry:erase(-wlback)
	self:render()
end--}}}

function Screen:word_left()--{{{
	local wlback, wlfwd = self.entry:wordlen()
	self.entry:move(-wlback)
	self:render()
end--}}}

function Screen:word_right()--{{{
	local wlback, wlfwd = self.entry:wordlen()
	self.entry:move(wlfwd)
	self:render()
end--}}}

local function history_save()--{{{
	if not self.entry:is_dirty() or self.entry:get() == "" then
		return
	end
	-- Suppress repeated lines from going into history
	if self.history:at_end() and self.entry:get() == self.history:get_current() then
		return
	end
	self.history:print(self.entry:get())
end--}}}

function Screen:submit()--{{{
	local line = selfentry:get()
	if #line > 0 then
		history_save()
		self.history:scroll_to(0)
		self.sb_at_end = true
		self.entry:clear()
		self:render()
		shell.eval(line)
	end
end--}}}

function Screen:history_backward()--{{{
	history_save()
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
end--}}}

function Screen:history_forward()--{{{
	history_save()
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
end--}}}

-- }}}

--[[ {{{ event names, old
event.add("startup",  startup)
event.add("keypress", self.keypress)
event.add("print",    function(...) M.debug("%1", table.concat({...}, " ")) end)

event.add("screen refresh",     M.render)
event.add("window goto",        M.goto)
event.add("entry submit",       M.submit)
event.add("entry backspace",    M.backspace)
event.add("buffer scroll up",   M.scroll_up)
event.add("buffer scroll down", M.scroll_down)
event.add("entry move left",    M.move_left)
event.add("entry move right",   M.move_right)
event.add("entry move home",    M.move_home)
event.add("entry move end",     M.move_end)
event.add("entry history prev", M.history_backward)
event.add("entry history next", M.history_forward)
event.add("entry delete word",  M.word_delete)
event.add("entry move left by word",  M.word_left)
event.add("entry move right by word", M.word_right)
}}}]]

return Screen
