require "moonshine.prelude"
require "moonshine.ui"

local loop = require "moonshine.loop"

emit "startup"
loop.run()
emit "shutdown"
