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
local log = require "moonshine.log"

local M = {}

local DEFAULT_CMD = "say"

local function invoke(name, text)
    local cmd = "cmd_" .. name
    if not _G[cmd] then
        require("moonshine.shell." .. name)
    end
    if not _G[cmd] then
        log.warning("Unknown command: /%s", name)
    end
    local ok, errmsg = pcall(_G[cmd], text)
    if not ok then
        log.critical("Error in command /%s: %s", name, errmsg)
    end
end

function M.execute(line)
    local name, pos = line:match("^/([%w_]+)%s*()")
    if name then
        local text = string.sub(line, pos)
        invoke(name, text)
    else
        invoke(DEFAULT_CMD, line)
    end
end

return M
