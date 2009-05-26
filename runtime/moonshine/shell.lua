-- This module provides an evaluator for moonshine commands.
-- Moonshine commands are the /foo bar baz strings that you 
-- type in the entry area.
--
-- shell.eval('/foo bar baz')
-- shell.call('foo', 'bar baz')
--
-- shell.register(name, {
--     func = function(text) assert(text == 'bar baz') end,
-- }
--
-- shell.require(name): shortcut for shell.register(name, require('moonshine.shell.' .. name))
--
-- TODO: document spec argument to shell.register.
local parseopt = require "moonshine.parseopt"
local M        = {}
local cmd      = {}

function M.call(name, arg)
	local func = cmd[name]
	if not func then
		local ok, errmsg = pcall(M.require, name)
		if ok then
			func = cmd[name]
		elseif not errmsg:match("module 'moonshine.shell." .. name .."' not found:") then
			run_hook('shell error', errmsg)
			return false
		end
	end

	if func then
		local ok, errmsg = pcall(func, arg)
		if not ok then
			run_hook('shell error', errmsg)
			return false
		else
			return true
		end
	else
		run_hook("unknown command", name, arg)
		return nil
	end
end

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

	return M.call(name, arg)
end

function M.register(name, def)
	local spec  = def.spec
	local run   = def.run or def.func

	assert(name, "name required")
	assert(run,  "run field required")

	if spec then
		local parser = parseopt.build_parser( unpack(spec) )
		cmd[name] = function(text)
			run( parser(text) )
		end
	else
		cmd[name] = run
	end
end

function M.require(name)
	local mod = require("moonshine.shell." .. name)
	M.register(name, mod)
	return mod
end

return M
