local core = require "moonshine.parseopt.core"
local NOARG, EATARG, STOP = core.NOARG, core.EATARG, core.STOP

return {
	boolean = function (val, _) return NOARG,  true          end,
	counter = function (val, c) return NOARG,  (c or 0) + 1  end,
	string  = function (val, _) return EATARG, tostring(val) end,
	number  = function (val, _) return EATARG, tonumber(val) end,
	table   = function (val, t)
		-- TODO: fixme
		return EATARG, t
	end,
	list    = function (val, l)
		if l then table.insert(l, val)
		else      l = { val } end
		return EATARG, l
	end,
}
