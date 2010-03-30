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

-- add a new Template.
function Template:define(name, code)
    local f = loadstring(parser.read(code))
    setfenv(f, self.env)
    self.env[name] = f
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

function Template:eval(text, ...)
    local f = loadstring(parser.read(text))
    setfenv(f, self.env)
    return f(...)
end

return Template
