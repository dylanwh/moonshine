require "bind" 
require "screen"
require "cmd"

function boot_hook()
	screen = Screen:new()
	screen:render()
	
	keypress_hook = screen:callback "keypress"
	--bind("^[[A", ui.move_up)
	--bind("^[[B", ui.move_down)
	bind("^[[C", screen:callback "move_right")
	bind("^[[D", screen:callback "move_left")
	bind("^[[5~", screen:callback "scroll_up")
	bind("^[[6~", screen:callback "scroll_down")
	bind("^?",  screen:callback "backspace")
	bind("^C", quit)
	bind("^X", quit)
	bind("^M", screen:callback ("send_line", eval))
	for i = 1, 9 do
		bind("^[" .. i, screen:callback("view", i))
	end
end

function quit_hook()
	screen:debug("Shutdown: %1", "bob")
end


