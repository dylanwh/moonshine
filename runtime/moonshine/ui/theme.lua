local format = require "moonshine.ui.format"

local M = {}

function M.apply(name)
    local theme = require("moonshine.ui.theme." .. name)
    theme.setup()
end

return M
