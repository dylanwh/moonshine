require "moonshine.prelude"
require "moonshine.ui"
require "moonshine.log"

local loop = require "moonshine.loop"

emit "startup"
loop.run()
emit "shutdown"
