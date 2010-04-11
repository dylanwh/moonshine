local screen = require "moonshine.ui.screen"
local format = require "moonshine.ui.format"

local M = {}

function M.setup()

    format.define_style('topic', 'white', 'blue')
    format.define_style('status', 'white', 'blue')
    format.define_style('status_bit', 'cyan', 'blue')

    format.define("timestamp", "$(date '%H:%M' $1)")
    format.define('clock', '$(timestamp $now)')

    format.define_style('username_bit', 'black2', 'default')
    format.define('username_bit', "$(style username_bit)<$^$1$(style username_bit)>$^")
    format.define("chat",    "$(timestamp $1) $(username_bit $2) $|$(markup $3)")
    format.define("public",  "$(chat $0)")
    format.define("private", "$(chat $0)")
    format.define("private_sent", "$(chat $0)")

    format.define('log_message', "$(timestamp $now) $|[$2] $3")

    -- note that functions passed to define() can only access lexical
    -- scope, not globals.
    format.define('view_info', function (key) return screen.view_info(key) end)
    format.define('prompt',  "[$(view_info name)] ")
    format.define('status_act', function()
        local acts = {}
        for i, v in screen.activity() do
            acts[i] = style("act_" .. v.level, v.index .. "")
        end
        if #acts > 0 then
            return status_bit('Act: ' .. concat_(',', acts))
        else
            return ""
        end
    end)

    format.define('topic', '$(style topic)$1')

    -- note the trailing space.
    format.define('status_bit',  '$(style status_bit $<[$(style status $1)] >)')
    format.define('status_time', '$(status_bit $(clock))')
    format.define('status_desc', '$(view_info index):$(view_info name)')
    format.define('status_view', '$(status_bit $(status_desc))')
    format.define('status',      '$(style status) $(status_time)$(status_view)$(status_act)')

    format.define_style('bold', 'white2', 'default')
    format.define('bold', '$(style bold $1)')
    format.define_style('blue', 'blue2', 'default')

    format.define('link', '$1')


    format.define('bip', '$(style blue $<-$(style bold "!")->)')

    format.define_style('green',   'green',  'default')
    format.define_style('greener', 'green2', 'default')
    format.define_style('userlist_bit', 'black2', 'default')
    format.define_style('userlist_txt', 'default', 'default')

    format.define('userlist_bit',  '$(style userlist_bit $<[$(style userlist_txt $1)]>)')
    -- $(userlist_head $room)
    format.define('userlist_head', "$(clock) $|$(userlist_bit $<$(style green 'Users') $(style greener $1)>)")

    -- $(userlist_foot $room $total_users $ops $halfops $voices $normal)
    format.define('userlist_foot', '$(clock) $bip $|Moonshine: $1: Total $2 nicks [$3 ops, $4 halfops, $5 voices, $6 normal]')
    format.define('userlist_flag', function (flag)
        if flag.founder     then return '&'
        elseif flag.op      then return '@'
        elseif flag.halfop  then return '%'
        elseif flag.voice   then return '+'
        else                     return ' '
        end
    end)

    -- $(userlist_item $flags $name)
    format.define('userlist_item', '$(userlist_bit $<$(userlist_flag $1)$2>)')
    format.define('userlist_line', '$(clock) $|$0')
end

return M
