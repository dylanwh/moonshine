local format = require "moonshine.ui.format"

local View = new "moonshine.object"

function View:__init(init)
    self._topic  = new "moonshine.ui.label"
    self._buffer = new "moonshine.ui.buffer"

    self._name         = assert(init.name, "name parameter is required")
    self._conversation = init.conversation

    if self._conversation and self._conversation:get_type() == 'chat' then
        self:update_topic( self._conversation:get_topic())
    else
        self:update_topic( '[' .. self:name() .. ']' )
    end
end

function View:update_topic(text)
    self._topic:set( format.apply('topic', text or 'NO TOPIC') )
end

function View:name()
    return self._name
end

function View:conversation()
    return self._conversation
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

function View:info(key)
    if key == 'name' then
        return self:name()
    else
        return ""
    end
end

return View
