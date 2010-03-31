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

local core  = require "moonshine.parseopt.core"
local lpeg  = require "lpeg"
local M     = {}

-- {{{ spec parsing stuff
local HINTS = { s = 'string', n = 'number', t = 'table', l = 'list', b = 'boolean', i = 'counter' }

local P, S, R       = lpeg.P, lpeg.S, lpeg.R
local C, Ct, Cc     = lpeg.C, lpeg.Ct, lpeg.Cc

local letter        = R ("az", "AZ") + S "_-"
local word          = letter ^ 1
local option_name   = C ( word )
local option_hint   = S "sntlbc" / HINTS
local option_rest   = (P "|" * option_name) ^ 0
local option_names  = Ct (option_name * option_rest)
local option        = option_names * ( (P "=" * option_hint) + Cc 'boolean' )

local function parse_spec (s)
    return lpeg.match(option, s)
end
-- }}}

function M.build_parser(spec)--{{{
    local hints = {}
    local alias = {}

    for _, x in ipairs(spec) do
        if type(x) == 'number' or x:match("^%d$") then
            hints[tonumber(x)] = true
        else
            local names, hint = parse_spec(x)
            local primary     = table.remove(names, 1)
            hints[primary]    = hint
            alias[primary]    = primary
            for _, name in ipairs (names) do
                alias[name] = primary
            end
        end
    end

    return function(text)
        local options = {}
        local args    = {}
        local function callback(name, value)
            local primary = alias[name or '']
            local hint    = hints[primary]
            if primary == nil then
                if hints[ #args + 1 ] then
                    table.insert(args, value)
                    return core.NOARG
                else
                    return core.STOP
                end
            elseif hint == 'boolean' then
                options[primary] = true
                return core.NOARG
            elseif hint == 'string' then
                options[primary] = value
                return core.EATARG
            elseif hint == 'number' then
                options[primary] = tonumber(value)
                return core.EATARG
            elseif hint == 'list' then
                if not options[primary] then
                    options[primary] = {}
                end
                table.insert(options[primary], value)
                return core.EATARG
            end
        end

        local rest = core.parse(text, callback)
        table.insert(args, rest)
        return options, unpack(args)
    end
end--}}}

return M
