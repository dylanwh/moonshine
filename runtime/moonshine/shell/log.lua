local M = {}
local log = require "moonshine.log"

M.name = "log"
M.spec = { "level|l=s" }

function M.run(o, text)
    log(o.level or 'DEBUG', text)
end

return M
