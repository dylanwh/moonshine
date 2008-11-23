local signal = require "moonshine.signal"
local shell  = require "moonshine.shell"
local term   = require "moonshine.ui.term"

local Entry     = require "moonshine.ui.entry"
local Buffer    = require "moonshine.ui.buffer"
local Statusbar = require "moonshine.ui.statusbar"
local Window    = require "moonshine.ui.window"


local M = {
	entry     = Entry:new(),
	status    = Statusbar:new(),
	history   = Buffer:new(),
	sb_at_end = true,
	windows   = {},
	window    = nil,
	status_bits = {
		"status_time",
		"status_current_window",
		"status_activity",
	},
}

local function startup()--{{{
	M.status:set( term.format("%{topic}Status bar goes here", {}))
	M.add( Window:new("status") )
	M.render()
end--}}}

function M.status_time()--{{{
	return os.date("%H:%M")
end--}}}

function M.status_current_window()--{{{
	if M.window then
		return (M.window.pos .. ":" .. (M.window.name or '???'))
	else
		return nil
	end
end--}}}

function M.status_activity()--{{{
	local actvals = { }
	if M.window then
		M.window.activity = 0
	end
	for i, window in pairs(M.windows) do
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

function M.update_status()--{{{
	local statusbuf = ""
	for i, bit in ipairs(M.status_bits) do
		local result = M[bit]()
		if result then
			statusbuf = statusbuf .. term.format("%{statusbracket} [%{statustext}%1%{statusbracket}]", {result})
		end
	end
	M.status:set(statusbuf)
end--}}}

function M.print(fmt, ...)--{{{
	M.window:print(fmt, ...)
	M.render()
end--}}}

function M.debug(fmt, ...)--{{{
	M.window:print("%{blue}-%{white}!%{blue}- %{default}%|"..fmt, ...)
	M.render()
end--}}}

function M.add(win)--{{{
	table.insert(M.windows, win)
	if M.window == nil then
		M.window = win
	end
	win.pos = #M.windows
	win:set_topic("Moonshine - " .. (win.name or '???'));
	return win.pos
end--}}}

function M.remove(win)--{{{
	assert(win.pos ~= nil, "window has position")
	if M.window == win then
		M.window = M.windows[win.pos - 1]
		M.window:set_topic("Moonshine - " .. (win.name or '???'));
		M.entry:set_prompt("[" .. (M.window.name or '???' ) .. "] ")
	end
	table.remove(M.windows, win.pos)
	for i, w in ipairs(M.windows) do
		w.pos = i
	end
end--}}}

function M.view(n)--{{{
	n = tonumber(n)
	if M.windows[n] then
		M.window = M.windows[n]
		M.entry:set_prompt("[" .. (M.window.name or '???' ) .. "] ")
		M.window:activate()
		M.render()
		return true
	else
		return false, "invalid window index: " .. tostring(n)
	end
end--}}}

function M.render()--{{{
	local rows, cols = term.dimensions()
	local window = M.window
	local status = M.status
	local entry  = M.entry

	M.update_status()
	
	status:render(rows - 2)
	window.bufferdirty = true
	window:render(0, rows - 3)
	entry:render()

	term.refresh()
end--}}}

-- {{{ functions for keybindings.
function M.keypress(key)--{{{
	M.entry:keypress(key)
	M.render()
end--}}}

function M.move_left() --{{{
	M.entry:move(-1)
	M.render()
end--}}}

function M.move_right()--{{{
	M.entry:move(1)
	M.render()
end--}}}

function M.move_home()--{{{
	M.entry:move_to(0)
	M.render()
end--}}}

function M.move_end()--{{{
	M.entry:move_to(-1)
	M.render()
end--}}}

function M.scroll_up()--{{{
	M.window:scroll(5)
	M.render()
end--}}}

function M.scroll_down()--{{{
	M.window:scroll(-5)
	M.render()
end--}}}

function M.backspace()--{{{
	M.entry:erase(-1) 
	M.render()
end--}}}

function M.word_delete()--{{{
	local wlback, wlfwd = M.entry:wordlen()
	M.entry:erase(-wlback)
	M.render()
end--}}}

function M.word_left()--{{{
	local wlback, wlfwd = M.entry:wordlen()
	M.entry:move(-wlback)
	M.render()
end--}}}

function M.word_right()--{{{
	local wlback, wlfwd = M.entry:wordlen()
	M.entry:move(wlfwd)
	M.render()
end--}}}

local function history_save()--{{{
	if not M.entry:is_dirty() or M.entry:get() == "" then
		return
	end
	-- Suppress repeated lines from going into history
	if M.history:at_end() and M.entry:get() == M.history:get_current() then
		return
	end
	M.history:print(M.entry:get())
end--}}}

function M.submit()--{{{
	local line = M.entry:get()
	if #line > 0 then
		history_save()
		M.history:scroll_to(0)
		M.sb_at_end = true
		M.entry:clear()
		M.render()
		shell.eval(line)
	end
end--}}}

function M.history_backward()--{{{
	history_save()
	-- Are we just starting to scroll back?
	if not M.sb_at_end then
		M.history:scroll(1)
	end
	M.sb_at_end = false
	local v = M.history:get_current()
	if v then
		M.entry:set(v)
	end
	M.render()
end--}}}

function M.history_forward()--{{{
	history_save()
	if M.history:at_end() then
		M.entry:clear()
		M.sb_at_end = true
	else
		M.history:scroll(-1)
		local v = M.history:get_current()
		if v then
			M.entry:set(v)
		end
	end
	M.render()
end--}}}
-- }}}

signal.add("startup",  startup)
signal.add("render",   M.render)
signal.add("keypress", M.keypress)
signal.add("print",    function(...) M.debug("%1", table.concat({...}, " ")) end)

signal.add("key return",    M.submit)
signal.add("key backspace", M.backspace)
signal.add("key up",        M.scroll_up)
signal.add("key down",      M.scroll_down)
signal.add("key left",      M.move_left)
signal.add("key right",     M.move_right)
signal.add("key home",      M.move_home)
signal.add("key end",       M.move_end)
signal.add("key page_up",   M.history_backward)
signal.add("key page_down", M.history_forward)
signal.add("key word_delete", M.word_delete)
signal.add("key word_left",   M.word_left)
signal.add("key word_right",  M.word_right)
signal.add("view", M.view)

return M
