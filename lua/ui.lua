local Topic   = Topic
local Buffer  = Buffer
local Entry   = Entry
local refresh = refresh

module "ui"
topic   = Topic("Test")
buffer  = Buffer()
entry   = Entry()

function print(fmt, ...)
	local s = buffer.format(fmt, arg)
	buffer:print(s)
end

function render()
	topic:render()
	buffer:render()
	entry:render()
	refresh()
end

function keypress(key)
	entry:keypress(key)
	render()
end

function key_left() 
	entry:move(-1)
	render()
end

function key_right()
	entry:move(1)
	render()
end

function key_backspace()
	entry:erase(-1) 
	render()
end

function key_home()
	entry:move_to(-1)
	render()
end

function key_end()
	entry:move_to(1)
	render()
end
