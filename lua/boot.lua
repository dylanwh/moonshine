-- Eventually, with a saner keybinding system (bd_?)
-- bind() and unbind() can be C primitives.

do
	local keys = {}
	local last_key = nil

	function bind(spec, f)
		local k = keys
		local function index(s, i)
			return s:sub(i+1, i+1)
		end
		-- make_keyspec translates ^A into 0, etc.
		spec = make_keyspec(spec)
		last = spec:len() - 1
		for i = 0, last, 1 do
			local x = index(spec, i)
			if i == last then
				k[x] = f
			else
				k[x] = {}
				k = k[x]
			end
		end
	end

	function on_input(key)
		if type(last_key) == 'table' then
			last_key = last_key[key]
		else
			last_key = keys[key]
			if not last_key then
				on_keypress(key)
			end
		end
		if type(last_key) == 'function' then
			last_key()
			last_key = nil
		end
	end

	function on_input_reset()
		last_key = nil
	end

	function on_keypress(key)
		-- entry:keypress(key)
	end
end

bind("^[[D", function () print("left") end)
bind("^C", os.exit)
bind("·", function () on_input(".") end)


