local M = {}
local log = require "moonshine.log"

M.name = "log"
M.spec = { "level|l=s" }

function M.func(o, text)
    log(o.level or 'DEBUG', text)
end

return M
