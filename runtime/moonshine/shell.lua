-- this module provides an evaluator for moonshine commands.
-- Moonshine commands are the /foo bar baz strings that you 
-- type in the entry area.
--
-- Commands are signals.
-- For instance, /msg  --target=bob
-- is: signal.emit("command msg", "--target=bob")
--
-- moonshine.parseopt provides a way of parsing options in the argument 
-- to command signals.
--
-- shell.define provides some sugar around all of this.

local signal   = require "moonshine.signal"
local parseopt = require "moonshine.parseopt"
local PREFIX   = "shell "
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

	if not signal.emit(PREFIX .. name, arg) then
		signal.emit("unknown command", name, arg)
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
		signal.add(PREFIX .. name, function(text)
			action( parser(text) )
		end)
	else
		signal.add(PREFIX .. name, action)
	end
end

function M.require(name)
	local mod = require("moonshine.shell." .. name)
	mod.name = name
	M.define(mod)
end

M.define {
	name = "quit",
	action = os.exit,
}

return M
