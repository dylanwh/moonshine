local format = require "moonshine.ui.format"

local View = new "moonshine.object"

function View:__init(init)
    self._topic  = new "moonshine.ui.label"
    self._buffer = new "moonshine.ui.buffer"

    self._name = assert(init.name, "name parameter is required")
    self._conv = init.conv

    self:set_topic( '[' .. self:get_name() .. ']' )
end

function View:set_topic(text)
    self._topic:set( format.apply('topic', text) )
end

function View:get_name()
    return self._name
end

function View:get_conv()
    return self._conv
end

function View:add_message(name, ...)
    self._buffer:print(format.apply(name, ...))
end

function View:print(text, ...)
    self._buffer:print(format.eval(text, ...))
end

function View:render(t, b)
    self._topic:render(t)
    self._buffer:render(t+1, b)
end

return View
