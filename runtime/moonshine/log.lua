local M = {}
local levels = { "message", "warning", "critical", "error", "debug" }

local log_core = require "moonshine.log.core"
local alias = { warn = 'warning', crit = 'critical', err = 'error', msg = 'message' }

log_core.set_default_handler(function (...) emit("log", ...) end)

function M.print(level, fmt, ...)
	log_core.print(M.DOMAIN, alias[level:lower()] or level or 'debug', string.format(fmt, ...))
end

for i, level in ipairs(levels) do
	M[level] = function(fmt, ...)
		return M.log(level, fmt, ...)
	end
end

return setmetatable(M, { __call = function(M, ...) M.print(...) end })
