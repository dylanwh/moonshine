module ("ui", package.seeall)

windows = {
	{ topic   = Topic("Test"), buffer  = Buffer() },
	{ topic   = Topic("Test2"), buffer  = Buffer() },
}

topic = windows[1].topic
buffer = windows[1].buffer
entry = Entry()

function view(i)
	if windows[i] then
		topic = windows[i].topic
		buffer = windows[i].buffer
		render()
	end
end

function print(fmt, ...)
	local s = Buffer.format(tostring(fmt), arg)
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

function scroll_up()
	buffer:scroll(5)
	render()
end

function scroll_down()
	buffer:scroll(-5)
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
