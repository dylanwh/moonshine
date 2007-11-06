module ("ui", package.seeall)
require "window"

windows = {
	Window:new(),
	Window:new(),
}

windows[1]:set_topic("Window 1")
windows[2]:set_topic("Window 2")

window = windows[1] 
entry  = Entry.new()

function view(i)
	if windows[i] then
		window = windows[i]
		render()
	else
		ui.print("Unknown window: %1", i)
	end
end

function print(fmt, ...)
	window:print(fmt, unpack(arg))
	render()
end

function debug(fmt, ...)
	window:print("[debug] %| "..fmt, unpack(arg))
	render()
end

function render()
	window:render()
	entry:render()
	refresh()
end

function keypress(key)
	entry:keypress(key)
	render()
end

function scroll_up()
	window:scroll(5)
	render()
end

function scroll_down()
	window:scroll(-5)
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
