-- This module provides an evaluator for moonshine commands.
-- Moonshine commands are the /foo bar baz strings that you 
-- type in the entry area.
--
-- shell.eval('/foo bar baz')
-- shell.call('foo', 'bar baz')
--
-- shell.define(name, {
--     func = function(text) assert(text == 'bar baz') end,
-- }
--
-- shell.require(name): shortcut for shell.define(name, require('moonshine.shell.' .. name))
--
-- TODO: document spec argument to shell.define.
local parseopt = require "moonshine.parseopt"

local M        = {}
local CMD      = {}

function M.call(name, arg)
    local func = CMD[name]

    if not func then
        local ok, errmsg = pcall(M.require, name)
        if ok then
            func = CMD[name]
        elseif not errmsg:match("module 'moonshine.shell." .. name .."' not found:") then
            run_hook('shell error', errmsg)
            return false
        end
    end

    if func then
        local ok, errmsg = pcall(func, arg)
        if not ok then
            run_hook('shell error', errmsg)
            return false
        else
            return true
        end
    else
        run_hook("unknown command", name, arg)
        return nil
    end
end

function M.eval(line)
    local name, pos = line:match("^/([%w_]+)()")
    local arg
    if name then
        name = name:lower()
        arg  = line:sub(pos+1)
    else
        name = "say"
        arg  = line
    end

    return M.call(name, arg)
end

function M.define(name, option)
    local spec = option.spec
    local run  = option.run

    assert(name, "name required")
    assert(run,  "run field required")

    if spec then
        local parser = parseopt.build_parser( unpack(spec) )
        CMD[name] = function(text)
            run( parser(text) )
        end
    else
        CMD[name] = run
    end
end

function M.require(name)
    local mod = require("moonshine.shell." .. name)
    M.define(name, mod)
    return mod
end

return M
