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

function Template:make(code, name)
    if type(code) == 'string' then
        return SAFE(setfenv(loadstring(parser.read(code), name), self.env))
    else
        if type(code) == 'function' then
            setfenv(code, self.env)
        end
        return SAFE(code)
    end
end

-- add a new Template.
function Template:define(name, code)
    self.env[name] = self:make(code, name)
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
