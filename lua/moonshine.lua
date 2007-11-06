require "bind" 
require "ui"
require "cmd"
require "config"
require "util"

server = {}

function connect_hook(host, port, fd, err)
	local function callback(h, event, ...)
		local f = _G["handle_"..event.."_hook"]
		if f then
			f(h, unpack(arg))
		else
			ui.debug("Let's just ignore that %1 handle event that just happend", event)
		end
	end
	if fd then
		ui.debug("Connected to %1:%2", host, port)
		local h = Handle.new(fd, callback)
		h:write("HAVER\tmoonshine\n")
		server[h] = {
			host = host,
			port = port,
			reader = LineReader.new(),
			window = ui.window,
		}
		ui.window.handle = h
	else
		ui.debug("Connection to %1:%2 failed: %3", host, port, err.message)
	end
end

function handle_read_hook(h, str)
	local reader = server[h].reader
	for i, line in ipairs(reader:read(str)) do
		line_hook(h, line)
	end
end

function line_hook(h, line)
	line = line:gsub("\r", "")
	local msg = string.split("\t", line)
	local f = _G[msg[1] .. "_hook"]
	if f then 
		f(h, msg)
	else
		ui.debug("Unknown haver thing: %1", msg[1])
	end
end

function HAVER_hook(h, msg)
	h:write("IDENT\t"..os.getenv('USER').."\n")
	ui.debug("Identifying...")
end

function HELLO_hook(h, name)
	ui.debug("Logged in.")
end

function IN_hook(h, msg)
	local _, room, user, type, msg = unpack(msg)
	ui.print("[%3] <%1> %|%2", user, msg, room)
end

function JOIN_hook(h, msg)
	ui.print("[%1 joined %2]", msg[3], msg[2])
end

function boot_hook()
	ui.render()
end

function quit_hook()
	ui.print("Shutdown: %1", "bob")
end

function connect(host, port)
	local function callback(fd, error)
		connect_hook(host, port, fd, error)
	end
	net.connect(host, port, callback)
end

