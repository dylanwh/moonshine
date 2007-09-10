-- GLOBALS: ui, on_keypress, on_resize.
--
-- This file defines a "ui" object that is a front-end for the buffer and entry
-- objects. It forwards most calls to either entry or buffer.
--
-- It also provides a :render() method that calls :render() on entry and buffer,
-- and finally the C routine refresh() to repaint the terminal.
--
-- We use the on_resize() hook to repaint the screen using ui:render()
-- We are guranteed that the current terminal size information is gathered by
-- the C-side before calling on_resize().
--
-- There are a few ui functions define UI:
--   left
--   right
--   backspace
-- which can be used like so:
-- bind("^[[D", ui.left)

declare "ui"
declare "on_keypress"
declare "on_resize"

ui = {
	topic  = Topic(""),
	buffer = Buffer(),
	entry = Entry(),
}

local function expose(object, field, method)
	object[method] = function(self, ...)
		local widget = self[field]
		local f = widget[method]
		return f(widget, unpack(arg))
	end
end

expose(ui, 'entry', 'keypress')
expose(ui, 'entry', 'move')
expose(ui, 'entry', 'erase')
expose(ui, 'entry', 'clear')
expose(ui, 'entry', 'get')

function ui:print(fmt, ...)
	local s = Buffer.format(fmt, arg)
	self.buffer:print(s)
end

function ui:set_topic(text)
	ui.topic:set(text)
	ui:render()
end

function ui:render()
	self.topic:render()
	self.buffer:render()
	self.entry:render()
	refresh()
end

function on_keypress(key)
	ui:keypress(key)
	ui:render()
end

function on_resize()
	ui:render()
end

function ui.left() 
	ui:move(-1)
	ui:render()
end

function ui.right()
	ui:move(1)
	ui:render()
end

function ui.backspace()
	ui:erase(-1) 
	ui:render()
end
