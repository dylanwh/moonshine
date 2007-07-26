-- Eventually, with a saner keybinding system (bd_?)
-- bind() and unbind() can be C primitives.

package.path = 'lua/?.lua'
require("input")

local entry = Entry.new()

function on_keypress(key)
	entry:keypress(key)
	entry:render()
end

local function left() 
	entry:move(-1)
	entry:render()
end

local function right()
	entry:move(1)
	entry:render()
end

local function backspace()
	entry:erase(-1) 
	entry:render()
end

bind("^[[D", left)
bind("^[[C", right)
bind("^?",  backspace)
bind("^C", app.exit)
bind("^M", function ()
	entry:clear()
	entry:render()
end)
