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
	if user == server.username then
		local window = Window:new()
		window:print("[You have joined %1]", room)
		table.insert(screen.windows, window)
		target = {
			type = 'room',
			name = room,
			server = server,
		}
		window.target = target
		server.windows.room[room] = window
		screen:view(#screen.windows)
	else
		local window = server.windows.room[room]
		window:print("[%1 joined %2]", user, room)
		screen:render()
	end
end

function part_hook(server, room, user)
	local window = server.windows.room[room]
	window:print("[%1 parted %2]", user, room)
	screen:render()
end

function public_message_hook(server, room, user, type, msg)
	local window = server.windows.room[room]

	if type == 'say' then
		window:print("<%1> %2", user, msg)
	elseif type == 'do' then
		window:print("*%1 %2", user, msg)
	else
		window:print("(%3)<%1> %2", user, msg, type)
	end
	screen:render()
end

function private_message_hook(server, user, type, msg)
	screen:print("[From %1] %2", user, msg)
end
