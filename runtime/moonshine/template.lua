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

local log    = require "moonshine.log"
local parser = require "moonshine.template.parser"

local Template = new "moonshine.object"
Template.const = {}
Template.env = {
    concat   = function (...) return table.concat({ ... }, "") end,
    concat_  = function (sep, list) return table.concat(list, sep) end,
    concat_0 = function (P) return table.concat(P, " ") end,
}
setmetatable(Template.env, { __index = function () return function () return "" end end })

function Template:__init()
    local base = self.__parent.env
    self.env = {
        const = function (C) return self.const[C] or C end,
        apply = function (name, ...) return self:apply(name, ...) end,
    }
    setmetatable(self.env, { __index = base })
end

local function SAFE(f)
    return function(...)
        local no_error, text = pcall(f, ...)
        if no_error then
            return text
        else
            log.critical("template/format error: %s", text)
            return ""
        end
    end
end

function Template:make(code, name, debug)
    if type(code) == 'string' then
        if debug then
            require "moonshine.log"
            log.debug("code for %s: %s", name, parser.read(code):gsub("\n", " "))
        end
        return SAFE(setfenv(loadstring(parser.read(code), name), self.env))
    else
        if type(code) == 'function' then
            setfenv(code, self.env)
        end
        return SAFE(code)
    end
end

-- add a new Template.
function Template:define(name, code, debug)
    self.env[name] = self:make(code, name, debug)
end

function Template:eval(code, ...)
    return self:make(code)(...)
end



-- apply a Template to a set of arguments.
-- recursive Template are not allowed.
function Template:apply(name, ...)
    local f = self.env[name]
    self.env[name] = nil
    local r = f(...)
    self.env[name] = f
    return r
end

return Template
