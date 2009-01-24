local signal = require "moonshine.signal"
local client = require "moonshine.net.client"

local M = {}
local S = {}

function M:new(host, port)
	local irc   = { host = host, port = port }
	local agent = client:new(host, port, function (event, ...)
		signal.emit("irc " .. event, irc, ...)
	end)
	agent:connect()

	irc.agent = agent

	setmetatable(irc, self)

	return irc
end

function S.connect(state)
	print("connected to", state.host)
end

function S.error(state)
end

function S.close(state)
end

function S.read(state, line)
end

function S.write(state)
end

function S.timeout(state)
end

function S.send(tag, msg)
	clients[tag]:write(msg .. "\r\n")
end


for k, f in pairs(S) do
	signal.add("irc " .. k, f)
end
