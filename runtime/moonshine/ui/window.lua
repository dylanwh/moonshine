local term   = require "moonshine.ui.term"

term.defcolor('topic', 'lightgray', 'blue')

local Window = new "moonshine.object"

Window:def_accessor('name')

function Window:__init(name)
    self:name(assert(type(name) == 'string', 'windows must have a name'))

    self._topic    = new("moonshine.ui.label")
    self._buffer   = new("moonshine.ui.buffer", 1014)
    self._activity = 0

    self:set_topic("Moonshine - A Haver Client")
end

function Window:set_topic(text)
    self._topic:set( term.getcolor('topic') .. text )
end

function Window:print(fmt, ...)
    self:actprint(1, fmt, ...)
end

function Window:actprint(activity, fmt, ...)
    local str = term.format(os.date("%H:%M ")..tostring(fmt), { ... })

    self:activity(activity)
    self._buffer:print(str)
end

function Window:render(top, bottom)
    local rows, cols = term.dimensions()

    self._topic:render(top)
    self._buffer:render(top + 1, bottom)
end

function Window:scroll(x)
    self._buffer:scroll(x)
end

function Window:activity(...)
    if select('#', ...) == 0 then
        return self._activity
    else
        local new = ...
        local old = self._activity
        self._activity = math.max( old, new )
        return self._activity
    end
end

function Window:activate()
    self._buffer:is_dirty(true)
    self:set_slot('activity', 0)
end

function Window:resize()
    self._buffer:is_dirty(true)
end

return Window
