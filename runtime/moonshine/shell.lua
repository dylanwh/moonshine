-- this module provides an evaluator for moonshine commands.
-- Moonshine commands are the /foo bar baz strings that you 
-- type in the entry area.

local parseopt = require "moonshine.parseopt"
local M        = {}
local cmd      = {}

function M.call(name, arg)--{{{
	local func = cmd[name]
	if not func then
		local ok, errmsg = pcall(M.require, name)
		if ok then
			func = cmd["cmd_" .. name]
		elseif not errmsg:match("module 'moonshine.shell." .. name .."' not found:") then
			emit('command error', errmsg)
			return false
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
		return nil
	end
end--}}}

function M.eval(line)--{{{
	local name, pos = string.match(line, "^/([%w_]+)()")
	local arg
	if name then
		name = string.lower(name)
		arg  = string.sub(line, pos+1)
	else
		name = "say"
		arg  = line
	end

	return M.invoke(name, arg)
end--}}}

function M.define(def)--{{{
	local name  = def.name
	local func  = def.func
	local spec  = def.spec

	assert(name, "name field required")
	assert(func, "func field required")

	if spec then
		local parser = parseopt.build_parser( unpack(spec) )
		cmd[name] = function(text)
			func( parser(text) )
		end
	else
		cmd[name] = func
	end
end--}}}

function M.require(name)--{{{
	local mod = require("moonshine.shell." .. name)
	mod.name = name
	M.define(mod)
	return mod
end--}}}

return M
