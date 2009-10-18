local term   = require "moonshine.ui.term"
local Window = new "moonshine.object"

Window.name     = accessor('_name')

function Window:__init()
    assert(self:name(), "name required")

    self._topic    = new("moonshine.ui.statusbar", "")
    self._buffer   = new("moonshine.ui.buffer", 1014)
    self._activity = 0

    self:set_topic("Moonshine - A Haver Client")
end

function Window:print(fmt, ...)
    self:actprint(1, fmt, ...)
end

function Window:actprint(activity, fmt, ...)
    local str = term.format(os.date("%H:%M ")..tostring(fmt), { ... })

    self._activity = math.max(self._activity, activity)
    self._buffer:print(str)
end

function Window:render(top, bottom)
    local rows, cols = term.dimensions()

    self._topic:render(top)
    if self._buffer:is_dirty() then
        self._buffer:render(top + 1, bottom)
    end
end

function Window:scroll(x)
    self._buffer:scroll(x)
end

function Window:set_topic(t)
    self._topic:set(term.format("%{topic}%1", { t }))
end

function Window:activate()
    self._buffer:is_dirty(true)
end

function Window:resize()
    self._buffer:is_dirty(true)
end

return Window
