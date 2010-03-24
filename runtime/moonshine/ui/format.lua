--[[
    $foo           is a macro call
    ${foo}         is a macro call
    ${foo bar baz} is a macro call with arguments.

    Arguments can be:
        * macro calls
        * sugar (described below)
        * quoted strings ("foo" or 'bar')
        * Any characters except $ { } and whitespace.

    Sugar:
        * $1         is short for $(param 1)
        * $|         is short for $(const '|')
        * $$         is short for $(const '$')
        * many other non-alphanumeric chars are also constants.

    Literals: Anything that does not begin with a '$' is a literal.
]]
require "moonshine.prelude" -- for new()

-- {{{ lpeg imports
local lpeg          = require "lpeg"
local P, R, S, V    = lpeg.P, lpeg.R,  lpeg.S,  lpeg.V
local C, Ct, Cc, Cs = lpeg.C, lpeg.Ct, lpeg.Cc, lpeg.Cs
local match         = lpeg.match
-- }}}

local letter  = R('AZ','az') + '_'
local digit   = R '09'
local space   = S " \t\r\n"

local number  = digit^1
              + digit^1 * '.' * digit^1
              + '.' * digit^1
local word    = letter * (letter + digit)^0
local quote   = function(chr)
    local any = (P ("\\" .. chr) / chr)
              + (P "\\\\" / "\\")
              + (P(1) - chr)
    return chr * Cs( any^0 ) * chr
end

local quoted  = quote([["]]) + quote([[']])
local name    = C( word )

local literal = C( (P(1) - '$')^1 )
local param   = '$'  * Ct( Cc "param" * (digit / tonumber) )
local const  = '$'  * Ct( Cc "const" * C(P(1)))
local junk    = C((S "~!%@#^&*-+=/\\,.|<>[]():;`"^1 + digit + letter)^1)

-- {{{ grammar
local format = P {
    'top',
    top      = Ct( Cc "top" * (V 'escape' + literal)^1 ) * -1,
    escape   = V 'macro' + param + const + ('$' * Ct(name)),
    macro    = '${' * space^0 * Ct(name * (space^1 * V 'tokens')^-1) * space^0 * '}',
    tokens   = V 'token' * (space^1 * V 'token') ^ 0,
    token    = (number / tonumber) + quoted + junk + V 'escape'
}
--}}}

local function read_str(str) return match(format, str) end

local function read_ast(ast)
    if type(ast) == 'table' then
        local op = table.remove(ast, 1)
        if op == 'top' then
            local code = "local P = { ... }\nreturn %s"
            local args = {}
            for i, x in ipairs(ast) do
                args[i] = read_ast(x)
            end
            return string.format(code, table.concat(args, " .. "))
        elseif op == 'const' then
            return 'C[' .. read_ast(ast[1]) .. ']'
        elseif op == 'param' then
            if ast[1] == 0 then
                return '_all(P)'
            else
                return 'P[' .. read_ast(ast[1]) .. ']'
            end
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

local function read_lua(env, lua)
    local f = loadstring(lua)
    setfenv(f, env)
    return f
end

local Format = new "moonshine.object"

function Format:__init()
    self.env   = { }
    self.env.C = {
        ['$'] = '$',
    }
    self.env._all = function(P)
        return table.concat(P, " ")
    end

    setmetatable(self.env,   { __index = function () return function () return "" end end })
    setmetatable(self.env.C, { __index = function () return "" end })
end

function Format:define(name, str)
    self.env.C[name] = tostring(str)
end

function Format:create(name, code)
    self.env[name] = read_lua(self.env, read_ast(read_str(code)))
end

function Format:apply(name, ...)
    return self.env[name](...)
end

print(read_ast(read_str('${msg $0}')))
local f = Format:new()
f:define('|', "pipe")
f:create("msg", "[$1]$| $2")
f:create("bmsg", "<b>${msg $0}</b>")
print (f:apply("bmsg", "dylan", "hello, world!"))

while true do
    local line = io.read()
    print(read_ast(read_str(line)))
end

return Format
