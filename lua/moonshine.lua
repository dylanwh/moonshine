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
	bind("^[[H", screen:callback "move_home")
	bind("^[[F", screen:callback "move_end")
	bind("^[[C", screen:callback "move_right")
	bind("^[[D", screen:callback "move_left")
	bind("^[[5~", screen:callback "scroll_up")
	bind("^[[6~", screen:callback "scroll_down")
	bind("^[[1;5D", screen:callback "word_left")
	bind("^[[1;5C", screen:callback "word_right")
	bind("^W", screen:callback "word_delete")
	bind("^?",  screen:callback "backspace")
	bind("^H",  screen:callback "backspace")
	bind("^C", quit)
	bind("^X", quit)
	bind("^L", force_resize)
	bind("^M", screen:callback ("send_line", eval))
	bind("\t", function () end)

	for i = 1, 9 do
		bind("^[" .. i, screen:callback("view", i))
	end

	screen:print("%{notice}You might consider typing:")
	screen:print("/connect")
	screen:print("/join main")
end

function resize_hook()
	screen:render()
end

function quit_hook()
	screen:debug("Shutdown: %1", "bob")
end

function join_hook(server, room, user)
	if user == server.username then
		local window = Window:new {
			type = 'room',
			name = room,
			server = server
		}
		window:print("[You have joined %1]", room)
		server.rooms[room] = { window = window }
		screen:add(window)
		screen:view(window.pos)
	else
		local window = server.rooms[room].window
		window:print("[%1 joined %2]", user, room)
		screen:render()
	end
end

function part_hook(server, room, user)
	local window = server.rooms[room].window
	window:print("[%1 parted %2]", user, room)
	screen:render()
end

function userlist_hook(server, room, users)
	local window = server.rooms[room].window
	window:print("[Users of %1: %2]", room, join(", ", users));
	screen:render()
end

function public_message_hook(server, room, user, type, msg)
	local window = server.rooms[room].window

	if type == 'say' then
		window:print("<%1> %|%2", user, msg)
	elseif type == 'do' then
		window:print("*%1 %|%2", user, msg)
	else
		window:print("(%3)<%1> %|%2", user, msg, type)
	end
	screen:render()
end

function public_message_sent_hook(server, room, user, type, msg)
	local window = server.rooms[room].window

	if type == 'say' then
		window:print("<%1> %|%2", user, msg)
	elseif type == 'do' then
		window:print("*%1 %|%2", user, msg)
	else
		window:print("(%3)<%1> %|%2", user, msg, type)
	end
	screen:render()
end

function query_hook(server, user)
	local window = Window:new {
		type = "user", 
		name = user,
		server = server
	}
	server.users[user] = { window = window }
	screen:view(screen:add(window))
end

function private_message_hook(server, user, type, msg)
	local window = server.users[user].window
	window:print("[From %1] %|%2", user, msg)
	screen:render()
end

function private_message_sent_hook(server, user, type, msg)
	local window = server.users[user].window
	window:print("[To %1] %|%2", user, msg)
end
