require "util"
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

	screen:print("%{notice}You might consider typing:")
	screen:print("/connect")
	screen:print("/join main")
end

function quit_hook()
	screen:debug("Shutdown: %1", "bob")
end

function join_hook(server, room, user)
	screen:print("[%1 joined %2]", user, room)
end

function part_hook(server, room, user)
	screen:print("[%1 parted %2]", user, room)
end

function public_message_hook(server, room, user, type, msg)
	if type == 'say' then
		screen:print("[%1] <%2> %3", room, user, msg)
	elseif type == 'do' then
		screen:print("[%1] *%2 %3", room, user, msg)
	else
		screen:print("[%1] (%4)<%2> %3", room, user, msg, type)
	end
end

function private_message_hook(server, user, type, msg)
	screen:print("[From %1] %2", user, msg)
end
