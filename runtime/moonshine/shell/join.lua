local protocol = require "moonshine.protocol"
local M = {}

M.spec = { "tag|t=s", 1}

function M.run(o, room)
    assert(o.tag)
    assert(room)
    protocol.join(o.tag, room)
end

return M
