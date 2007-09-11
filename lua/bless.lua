
function bless(t)
	local mt = getmetatable(t)
	if not mt then
		mt = {}
		setmetatable(t, mt)
	end
	mt.__call = function(t, ...)
		return t.new(unpack(arg))
	end
end

bless(Topic)
bless(Entry)
bless(Buffer)
