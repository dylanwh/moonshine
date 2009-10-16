local M = {}
local levels = { "message", "warning", "critical", "error", "debug" }

local log_core = require "moonshine.log.core"
local alias = { warn = 'warning', crit = 'critical', err = 'error', msg = 'message' }

function M.install()
    log_core.set_default_handler(function (...) run_hook("log", ...) end)
end

function M.print(level, fmt, ...)
    assert(level)
    assert(fmt)
    local ok, str = pcall(string.format, fmt, ...)
    if not ok then
        error(str, 3)
    else
        log_core.print(M.DOMAIN, alias[level:lower()] or level, str)
    end
end

for i, level in ipairs(levels) do
    M[level] = function(fmt, ...)
        return M.print(level, fmt, ...)
    end
end

return setmetatable(M, { __call = function(M, ...) M.print(...) end })
