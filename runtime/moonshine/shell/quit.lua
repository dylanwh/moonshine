local M    = {}
local loop = require "moonshine.loop"

M.spec = {}

function M.func()
    loop.quit()
end

return M
