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

	local func = _G["cmd_" .. name]
	if not func then
		local ok, errmsg = pcall(M.require, name)
		if ok then
			func = _G["cmd_" .. name]
		elseif not errmsg:match("module 'moonshine.shell." .. name .."' not found:") then
			emit('command error', errmsg)
		end
	end

	if func then
		local ok, errmsg = pcall(func, arg)
		if not ok then
			emit('command error', errmsg)
			return false
		else
			return true
		end
	else
		emit("unknown command", name, arg)
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
	return mod
end

return M
