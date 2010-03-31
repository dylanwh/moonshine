local log    = require "moonshine.log"
local parser = require "moonshine.template.parser"

local Template = new "moonshine.object"
Template.const = {}
Template.env = {
    concat   = function (...) return table.concat({ ... }, "") end,
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

function Template:make(code)
    if type(code) == 'string' then
        return setfenv(loadstring(parser.read(code)), self.env)
    else
        if type(code) == 'function' then
            setfenv(code, self.env)
        end
        return function(...)
            local no_error, text = pcall(code, ...)
            if no_error then
                return text
            else
                log.critical("template/format error: %s", text)
                return ""
            end
        end
    end
end

-- add a new Template.
function Template:define(name, code)
    self.env[name] = self:make(code)
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
