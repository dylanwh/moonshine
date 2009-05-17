local make_keyspec  = require("moonshine.ui.term").make_keyspec
local Object = require "moonshine.object"
local Tree   = require "moonshine.tree"
local screen = require "moonshine.ui.screen"
local term   = require "moonshine.ui.term"
local event  = require "moonshine.event"

local M = {}

local mapping = Tree.new()
local keybuf  = ""

local on_keypress = nil -- screen:callback "keypress"

function M.bind(spec_, name, ...)
	local spec = term.make_keyspec(spec_)
	local extra = { ... }
	local cb = function()
		M.invoke(name, unpack(extra))
	end

	mapping:insert(spec, cb)

	local found_value = mapping:find(spec)
end

function M.keypress(key)--{{{
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
		keybuf = ""
	else
		-- prefix of something, keep going
	end
end--}}}

function M.invoke(name, ...)
	local funcname = "kb_" .. name:gsub("[^a-zA-Z_]", "_")
--	_G[funcname](...)
end

event.add("input", M.keypress)
return M
