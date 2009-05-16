-- this module provides an evaluator for moonshine commands.
-- Moonshine commands are the /foo bar baz strings that you 
-- type in the entry area.
--
-- Commands are function calls in the global namespace.
-- For instance, /msg  --target=bob
-- is: cmd_msg("--target=bob")

local parseopt = require "moonshine.parseopt"
local M        = {}

function M.eval(line)
	local name, pos = string.match(line, "^/([%w_]+)()")
	local arg
	if name then
		name = string.lower(name)
		arg  = string.sub(line, pos+1)
	else
		name = "say"
		arg  = line
	end

	func = _G["cmd_" .. name]
	if func then
		func(arg)
	else
		unknown_command_hook(name, arg)
	end
end

function M.define(def)
	local name   = def.name
	local action = def.action
	local spec   = def.spec

	assert(name,   "name field required")
	assert(action, "action field required")

	if spec then
		local parser = parseopt.build_parser( unpack(spec) )
		_G["cmd_" .. name] = function(text)
			action( parser(text) )
		end
	else
		_G["cmd_" .. name] = action
	end
end

function M.require(name)
	local mod = require("moonshine.shell." .. name)
	mod.name = name
	M.define(mod)
end

return M
