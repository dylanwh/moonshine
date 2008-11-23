local Window = require "moonshine.ui.window"
local screen = require "moonshine.ui.screen"

local M = {}

M.spec = { 1, 2, 3 }

function M.action(_, cmd, ...)
	print(cmd)
	if M[cmd] then
		M[cmd](...)
	end
end

function M.new(mode)
	local win = Window:new("new")
	local n = screen.add(win)
	if mode ~= 'hidden' then
		screen.view(n)
	end
end

function M.goto(n)
	print("goto", n)
	screen.view(n)
end

return M
