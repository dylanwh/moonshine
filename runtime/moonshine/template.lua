--[[
    %foo           is a macro call
    %{foo}         is a macro call
    %{foo bar baz} is a macro call with arguments.

    Arguments can be:
        * macro calls
        * sugar (described below)
        * quoted strings ("foo" or 'bar')
        * bare words (foo, foo123, foo_123, foo_bar, _)
        * numbers (5, 8.4, .3)

    Sugar:
        * %1         is short for %{param 1}
        * %%         is a literal '%'
        * %|         is short for %{const '|'}
        * %(TEXT)    is special, it causes TEXT to be evaluated as if it was outside of any macro call.

    Literals: Anything that does not begin with a '%' is a literal.
]]
require "moonshine.prelude" -- for new()

-- {{{ lpeg imports
local lpeg          = require "lpeg"
local P, R, S, V    = lpeg.P, lpeg.R,  lpeg.S,  lpeg.V
local C, Ct, Cc, Cs = lpeg.C, lpeg.Ct, lpeg.Cc, lpeg.Cs
local match         = lpeg.match
-- }}}

-- {{{ grammar
local letter  = R('AZ','az') + '_'
local digit   = R '09'
local space   = S " \t\r\n"
local sp0     = space^0
local sp1     = space^1

local number  = C(digit^1 * '.' * digit^1)
              + C(digit^1)
              + C('.' * digit^1)
local word    = letter * (letter + digit)^0
local quote   = function(chr)
    local any = (P ("\\" .. chr) / chr)
              + (P "\\\\" / "\\")
              + (P(1) - chr)
    return chr * Cs( any^0 ) * chr
end

local quoted  = quote([["]]) + quote([[']])
local name    = C( word )
local literal = C( (P(1) - '%' )^1 )

local top   = {TOP=1} -- special value to represent "top" opcode.
local eval  = {EVAL= 1} -- same, for code injection.
local template = P {
    'top',
    top      = Ct( Cc(top) * (literal + V 'escape')^1 ) * -1,
    inject   = '(' * sp0 * C(((1 - S '()') + V 'inject')^0) * sp0 * ')',
    escape   = V 'macro' + V 'eval' + V 'param' + V 'const',
    eval     = '%'  * Ct( Cc(eval) * V 'inject' ),
    param    = '%'  * Ct( Cc 'param' * (digit^1 / tonumber) ),
    const    = '%%' * Cc '%'
             + '%'  * Ct( Cc 'const' * C(P(1)) ),
    macro    = '%{' * sp0 * '}' * Cc ''
             + '%{' * sp0 * Ct(name * (sp1 * V 'tokens')^-1) * sp0 * '}'
             + '%{' * sp0 * Ct(Cc "apply" * V 'tokens') * sp0 * '}'
             + '%'  * Ct(name),
    tokens   = V 'token' * (sp1 * V 'token') ^ 0,
    token    = V 'escape' + quoted + name + number/tonumber,
}
--}}}

local function read(str) return match(template, str) end

local function read_ast(ast)
    if type(ast) == 'table' then
        local op = table.remove(ast, 1)
        if op == top then
            local code = "local param = { ... }\nreturn %s"
            local args = {}
            for i, x in ipairs(ast) do
                args[i] = read_ast(x)
            end
            return string.format(code, table.concat(args, " .. "))
        elseif op == eval then
            local ast = read(ast[1])
            ast[1] = 'concat'
            return read_ast(ast)
        elseif op == 'param' and ast[1] == 0 then
            return '_concat(param)'
        elseif op == 'param' then
            return 'param[' .. read_ast(ast[1]) .. ']'
        elseif type(ast[1]) == 'table' and ast[1][1] == 'param' and ast[1][2] == 0 then
            return string.format("%s(...)", op)
        else
            local code = '%s(%s)'
            local args = {}
            for i, x in ipairs(ast) do
                args[i] = read_ast(x)
            end
            return string.format(code, op, table.concat(args, ", "))
        end
    elseif type(ast) == 'string' then
        if ast == "\n" then
            return '"\\n"'
        else
            return string.format('%q', ast)
        end
    elseif type(ast) == 'number' then
        return ast
    end
end

local Template = new "moonshine.object"

Template.const = {}
Template.env = {
    concat  = function (...) return table.concat({ ... }, " ") end,
    _concat = function (P) return table.concat(P, " ") end,
}
setmetatable(Template.env, { __index = function () return function () return "" end end })

function Template:__init()
    local base = self.__parent.env
    self.env = {
        const = function (C) return self.const[C] or '' end,
        apply = function (name, ...) return self:apply(name, ...) end,
    }
    setmetatable(self.env, { __index = base })
end

-- add a new Template.
function Template:define(name, code)
    local f = loadstring(read_ast(read(code)))
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

--print(read_ast(read("[debug]%| %{style debug}%1")))
--print(read_ast(read("%{debug %(we have %1 colors and %2 styles)}")))


return Template
