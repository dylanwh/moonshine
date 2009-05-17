local M = {}

local make_keyspec = require("moonshine.ui.term").make_keyspec
local Tree         = require "moonshine.tree"

local tree = Tree:new()

function M.bind(spec, name)
	tree:insert( make_keyspec(spec), name)
end

-- called by on_keypress_raw()
function M.keypress(k)
	-- FIXME
	-- else
	--     on_keypress(k)
	-- end
end


local function funcname(x)
	return "kb_" .. x:gsub("[^a-zA-Z_]", "_")
end

function M.invoke(name)
	_G[funcname(name)]()
end

function M.define(name, func)
	_G[funcname(name)] = func
end

return M
