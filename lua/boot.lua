-- Eventually, with a saner keybinding system (bd_?)
-- bind() and unbind() can be C primitives.

package.path = 'lua/?.lua'
require("input")

local entry = Entry.new()
local buffer = Buffer.new()

function on_keypress(key)
	entry:keypress(key)
	entry:render()
	app:refresh()
end

function on_resize()
	buffer:render()
	entry:render()
	app:refresh()
end

local function left() 
	entry:move(-1)
	entry:render()
	app:refresh()
end

local function right()
	entry:move(1)
	entry:render()
	app:refresh()
end

local function backspace()
	entry:erase(-1) 
	entry:render()
	app:refresh()
end

bind("^[[D", left)
bind("^[[C", right)
bind("^?",  backspace)
bind("^C", app.exit)
bind("^M", function ()
	buffer:print(entry:get())
	entry:clear()
	buffer:render()
	entry:render()
	app:refresh()
end)
