-- GLOBALS: bind, input_hook.
--
-- This hooks into input_hook and input_reset_hook.
-- It provides bind(keysequence, f) which causes f to be invoked when the
-- keysequence is seen in sucessive calls to input_hook().
-- input_reset_hook() is called as a sort-of safeguard against being stuck waiting
-- for input. 
--
-- Keys are UTF8 strings.
--
-- unbound keys are passed to keypress_hook().

local keys = {}
local last_key = nil

function bind(spec, f)
	local k = keys
	local function index(s, i)
		return s:sub(i+1, i+1)
	end
	-- make_keyspec translates ^A into 0, etc.
	local spec = make_keyspec(spec)
	local last = spec:len() - 1
	for i = 0, last, 1 do
		local x = index(spec, i)
		if i == last then
			k[x] = f
		else
			if k[x] == nil then
				k[x] = {}
			end
			k = k[x]
		end
	end
end

function input_hook(key)
	if type(last_key) == 'table' then
		last_key = last_key[key]
	else
		last_key = keys[key]
		if not last_key and keypress_hook then
			keypress_hook(key)
		end
	end
	if type(last_key) == 'function' then
		last_key()
		last_key = nil
	end
end
