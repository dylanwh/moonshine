local term      = require "moonshine.ui.term"

term.defcolor("status_bracket",   "cyan",          "blue")
term.defcolor("status_text",      "lightgray",     "blue")
term.defcolor("status_boring",    "cyan",          "blue")
term.defcolor("status_normal",    "white",         "blue")
term.defcolor("status_important", "brightmagenta", "blue")

local Statusbar = new "moonshine.object"
Statusbar:def_accessor('format')

function Statusbar:__init(screen)
    assert(screen)

    self:format("%{status_bracket}[%{status_text}$time%{status_bracket}]")
    self._label  = new "moonshine.ui.label"
    self._timer  = new("moonshine.timer", function()
        screen:render()
        return false
    end)
end

function Statusbar:render(row)
    self:update()
    self._label:render(row)
end

function Statusbar:update()
    local format = self:format():gsub("%%{([%w_]+)}", term.getcolor)
    local label  = format:gsub("$([%w_]+)", function(name)
        local bit = self["var_" .. name]
        local ok, result = pcall(bit, self)
        if ok and result then
            return tostring(result)
        else
            return ""
        end
    end)

    self._label:set(label)
end

function Statusbar:var_time()
    local now = os.date("*t")
    local next = {
        year  = now.year,
        month = now.month,
        day   = now.day,
        hour  = now.hour,
        min   = now.min + 1,
        sec   = 0,
        isdst = now.isdst
    }

    local wait = os.difftime(os.time(next), os.time(now)) * 1000

    -- Since lua's time is only accurate to the second, undershoot by 1s
    -- to make sure we redraw closer to the actual minute rollover.
    -- The edge case handling below will then have us poll every 100ms
    -- until the minute rolls over.
    wait = wait - 1000

    if wait <= 0 or wait > 59 * 1000 then
        -- We've hit some kind of weird edge case. Try again in 100ms
        wait = 100
    end

    self._timer:schedule(wait)

    return os.date("%H:%M")
end

return Statusbar
