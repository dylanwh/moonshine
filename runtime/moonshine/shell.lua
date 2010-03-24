-- This module provides an evaluator for moonshine commands.

local Shell = new "moonshine.object"

function Shell:__init(namespace)
    self._namespace = namespace or 'moonshine.shell'
    self._command   = {}
end

function Shell:bind(name, proto)
    local cmd = proto:new()
    self._command[name] = cmd
end

function Shell:bind_function(name, func)
    local cmd = new "moonshine.cmd"
    function cmd:parse(text) return text end
    function cmd:run(text) func(text) end

    self._command[name] = cmd
end

function Shell:eval(line)
    local name, pos = line:match("^([%w_]+)()")
    local text
    if name then
        name = name:lower()
        text = line:sub(pos+1)
    else
        name = 'say'
        text = line
    end

    return self:_call(name, text)
end

function Shell:_call(name, text)
    local cmd = self._command[name] or self:_autoload(name)
    if cmd then
        cmd:run(cmd:parse(text))
        return true
    else
        return false
    end
end

function Shell:_autoload(name)
    if self._namespace then
        local mod = self._namespace .. '.' .. name
        local ok, class = pcall(require, name)
        if ok then
            self:bind(name, class)
        end
        return self._command[name]
    end
end


return Shelk
