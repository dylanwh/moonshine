local M = {}

local make_keyspec = require("moonshine.ui.term").make_keyspec
local Tree         = require "moonshine.tree"
local screen       = require "moonshine.ui.screen.main"

local mapping = Tree:new()
local keybuf  = ""

function M.bind(spec_, name, ...)--{{{
	local spec = make_keyspec(spec_)
	local extra = { ... }
	local cb = function()
		M.invoke(name, unpack(extra))
	end

	mapping:insert(spec, cb)

	local found_value = mapping:find(spec)
end--}}}

function M.process(key)--{{{
	keybuf = keybuf .. key

	local found_key, func, index, dirn = mapping:find_near(keybuf)

	if found_key == nil then
		-- No keys defined??
		return
	end

	if dirn == 0 then
		func()
		keybuf = ""
		return
	elseif dirn > 0 then
		index = index + 1
		found_key, func = mapping:lookup_index(index)
	end

	if found_key == nil then
		-- should only ever happen if there are no keys registered at all
		found_key = ""
	end

	if string.sub(found_key, 1, string.len(keybuf)) ~= keybuf then
		-- not a prefix of anything
		-- XXX: deliver this to screen somehow
		emit("keypress", key)
		keybuf = ""
	else
		-- prefix of something, keep going
	end
end--}}}

local function funcname(x)
	return "kb_" .. x:gsub("[^a-zA-Z_]", "_")
end

function M.invoke(name)
	local func = _G[funcname(name)]
	if func then 
		func()
	end
end

function M.define(name, func)
	_G[funcname(name)] = func
end

return M
