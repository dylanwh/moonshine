--[[ vim: set ft=lua sw=4 ts=4 expandtab:
-   Moonshine - a Lua-based chat client
-
-   Copyright (C) 2010 Dylan William Hardison
-
-   This file is part of Moonshine.
-
-   Moonshine is free software: you can redistribute it and/or modify
-   it under the terms of the GNU General Public License as published by
-   the Free Software Foundation, either version 3 of the License, or
-   (at your option) any later version.
-
-   Moonshine is distributed in the hope that it will be useful,
-   but WITHOUT ANY WARRANTY; without even the implied warranty of
-   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-   GNU General Public License for more details.
-
-   You should have received a copy of the GNU General Public License
-   along with Moonshine.  If not, see <http://www.gnu.org/licenses/>.
]]

local M        = {}
local log_core = require "moonshine.log.core"
local alias    = { warn = 'warning', crit = 'critical', err = 'error', msg = 'message' }

M.set_default_handler = log_core.set_default_handler

function M.print(level, fmt, ...)
    assert(level)
    assert(fmt)

    local ok, str = pcall(string.format, fmt, ...)
    if not ok then
        log_core.print(M.DOMAIN, "critical", string.format("string.format(%s) error: %s", fmt, str))
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
