local M        = {}
local log_core = require "moonshine.log.core"
local alias    = { warn = 'warning', crit = 'critical', err = 'error', msg = 'message' }

M.set_default_handler = log_core.set_default_handler

function M.print(level, fmt, ...)
    assert(level)
    assert(fmt)

    local ok, str = pcall(string.format, fmt, ...)
    if not ok then
        log_core.print(M.DOMAIN, "error", string.format("string.format(%s) error: %s", fmt, str))
    else
        log_core.print(M.DOMAIN, alias[level:lower()] or level, str)
    end
end

do
    local levels   = { "message", "warning", "critical", "error", "debug" }
    for i, level in ipairs(levels) do
        M[level] = function(fmt, ...)
            return M.print(level, fmt, ...)
        end
    end
end

return setmetatable(M, { __call = function(M, ...) M.print(...) end })
