local Parseopt    = require "moonshine.parseopt" 
local M           = { }
local commands    = { }
local default_cmd = "say"

function M.eval(line)
	local name, pos = string.match(line, "^[/.]([%w_]+)()")
	local arg
	if name then
		arg = string.sub(line, pos+1)
	else
		name = default_cmd
		arg = line
	end

	local cmd = commands[name]
	if cmd then
		assert(cmd.exec, "commands must have exec functions!")
		assert(cmd.parse, "commands must have parse functions!")
		return cmd.exec( cmd.parse(arg) )
	else
		error("unknown shell command: " .. name)
	end
end

function M.register(name, cmd)
	assert(cmd.exec, "commands must have exec functions!")
	assert(cmd.parse, "commands must have parse functions!")

	if cmd.init then
		cmd.init(M, name)
	end
	commands[name] = cmd
end

function M.require(name)
	M.register(name, require("moonshine.shell." .. name))
end

function M.help(name)
	if not commands[name] then
		return "That command does not exist"
	elseif not commands[name].help then
		return "There is no help for that command"
	else
		return commands[name].help
	end
end

function M.build_parser(...)
	local parser = Parseopt:clone(...)
	return parser:callback("parse")
end

return M
