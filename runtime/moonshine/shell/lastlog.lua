local parseopt = require "moonshine.parseopt"
local log      = require "moonshine.log"
local screen   = require "moonshine.ui.screen"

local parse = parseopt.build_parser {
    "fixed|f",
    "clear|c",
}

function cmd_lastlog(text)
    local opt, pattern = parse(text)
    local view         = screen.current_view()

    if opt.clear then
        view:lastlog_clear()
        return
    end

    local f
    if opt.fixed then
        f = function(line)
            return string.find(line, pattern)
        end
    else
        f = function (line)
            return string.match(line, pattern)
        end
    end

    view:lastlog_search(f, 100)
    screen.render()
end
