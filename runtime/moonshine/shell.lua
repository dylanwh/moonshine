local command = require "moonshine.command"
local string  = string
local require = require
module "moonshine.shell"

function eval(line)
	local name, pos = string.match(line, "^/([%w_]+)()")
	local arg
	if name then
		name = string.lower(name)
		arg  = string.sub(line, pos+1)
	else
		name = "say"
		arg = line
	end

	if command[name] then
		local cmd = command[name]
		cmd.run( cmd.parse(arg) )
	end
end

function bind(name, cmd)
	command[name] = cmd
end
