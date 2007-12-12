protocols = {}
windows = {}

local function is_not_tag(h)
	for i, x in ipairs {"chat", "haver", "irc", "web", "ftp", "email", "mail", "www"} do
		if x == h then
			return true
		end
	end
	return false
end

function make_tag(hostname)
	assert(type(hostname) == 'string', "hostname must be a string!")
	local tag
	local hostname = split(".", hostname)
	if is_not_tag(hostname[1]) then
		tag = hostname[2]
	else
		tag = hostname[1]
	end

	local root, i = tag, 2
	while protocols[tag] do
		tag = root .. i
		i = i + 1
	end
	return tag
end

function init_hook()
	screen = Screen:clone()

	keypress_hook = screen:callback "keypress"
	bind("^[[A",    screen:callback "history_backward")
	bind("^[[B",    screen:callback "history_forward")
	bind("^[[H",    screen:callback "move_home")
	bind("^[[F",    screen:callback "move_end")
	bind("^[7~",    screen:callback "move_home")
	bind("^[8~",    screen:callback "move_end")
	bind("^[[C",    screen:callback "move_right")
	bind("^[[D",    screen:callback "move_left")
	bind("^[[5~",   screen:callback "scroll_up")
	bind("^[[6~",   screen:callback "scroll_down")
	bind("^[[1;5D", screen:callback "word_left")
	bind("^[[1;5C", screen:callback "word_right")
	bind("^W",      screen:callback "word_delete")
	bind("^?",      screen:callback "backspace")
	bind("^H",      screen:callback "backspace")
	bind("^C",      quit)
	bind("^L",      term.force_refresh)
	bind("^M",      screen:callback ("send_line", eval))
	bind("\t",      screen:callback "complete")

	for i = 1, 9 do
		bind("^[" .. i, screen:callback("view", i))
	end

	screen:print("%{notice}You might consider typing:")
	screen:print("/connect")
	screen:print("/join main")

	term.defcolor("statusbracket", "cyan", "blue")
	term.defcolor("statustext", "lightgray", "blue")
	term.defcolor("statusboring", "cyan", "blue")
	term.defcolor("statusnormal", "white", "blue")
	term.defcolor("statusimportant", "brightmagenta", "blue")
	term.defcolor("self", "white", "default")
	screen:render()

	for key in pairs(cmd) do
		screen.cmp:add( "/" .. key )
	end
end

function log_hook(domain, level, message)
	if screen then
		screen:debug("error: [%1] %2", domain, message)
	else
		error(message)
	end
end

function resize_hook()
	screen:render()
end

function quit_hook()
	screen:debug("Shutdown: %1", "bob")
end

function connect_hook(protocol)
	protocol.tag = make_tag(protocol.hostname)
	protocols[protocol.tag] = protocol
	windows[protocol.tag] = {
		room = protocol:magic_table(),
		user = protocol:magic_table(),
	}
	screen:debug("Connecting to %1 (%2:%3)", protocol.tag, protocol.hostname, protocol.port)
end

function connected_hook(protocol)
	screen:debug("Connected to %1 (%2:%3)", protocol.tag, protocol.hostname, protocol.port)
end

function disconnect_hook(protocol)
	protocols[protocol.tag] = nil
	windows[protocol.tag]   = nil
	screen:debug("Disconnected from %1 (%2:%3)", protocol.tag, protocol.hostname, protocol.port)
end


function topic_hook(protocol, room, topic)
	local tag    = protocol.tag
	local window = windows[tag]['room'][room]
	if window then
		window:set_topic(topic)
	end
end

function join_hook(protocol, room, user)
	local tag = protocol.tag
	if user == protocol.username then
		local window = Window:clone {
			type = 'room',
			name = room,
			protocol = protocol
		}
		window:print("[You have joined %1]", room)
		windows[tag]['room'][room] = window
		screen:add(window)
		screen:view(window.pos)
	else
		local window = windows[tag]['room'][room]
		window:print("[%1 joined %2]", user, room)
		screen:render()
	end
end

function part_hook(protocol, room, user)
	local tag = protocol.tag
	local window = windows[tag]['room'][room]
	if user == protocol.username then
		screen:remove(window)
	else
		window:print("[%1 parted %2]", user, room)
	end
	screen:render()
end

function userlist_hook(protocol, room, users)
	local tag = protocol.tag
	local window = windows[tag]['room'][room]
	window:print("[Users of %1] %|%2", room, join(", ", users));
	screen:render()
end

function public_message_hook(protocol, room, user, type, msg)
	local tag = protocol.tag
	local window = windows[tag]['room'][room]
	if not window then
		window = screen.window
	end

	if type == 'say' then
		window:actprint(2, "<%1> %|%2", user, msg)
	elseif type == 'do' then
		window:actprint(2, "* %1 %|%2", user, msg)
	else
		window:actprint(2, "(%3)<%1> %|%2", user, msg, type)
	end
	screen:render()
end

function public_message_sent_hook(protocol, room, user, type, msg)
	local tag = protocol.tag
	local window = windows[tag]['room'][room]

	if not window then
		window = screen.window
	end

	if type == 'say' then
		window:print("<%{self}%1%{default}> %|%2", user, msg)
	elseif type == 'do' then
		window:print("* %{self}%1%{default} %|%2", user, msg)
	else
		window:print("(%3)<%{self}%1%{default}> %|%2", user, msg, type)
	end
	screen:render()
end

function query_hook(protocol, user)
	local tag = protocol.tag
	local window = Window:clone {
		type = "user", 
		name = user,
		protocol = protocol
	}
	windows[tag]['user'][user] = window
	screen:view(screen:add(window))
end

function private_message_hook(protocol, user, type, msg)
	local tag = protocol.tag
	local window = windows[tag]['user'][user]
	if not window then
		window = screen.window
	end
	window:actprint(3, "[From %1] %|%2", user, msg)
	screen:render()
end

function private_message_sent_hook(protocol, user, type, msg)
	local tag = protocol.tag
	local window = windows[tag]['user'][user]
	if not window then
		window = screen.window
	end
	window:print("[To %{self}%1%{default}] %|%2", user, msg)
	screen:render()
end
