servers = {}

function server_name(hostname)
	local name
	local hostname = split(".", hostname)
	local function is_not_name(h)
		for i, x in ipairs {"chat", "haver", "irc", "web", "ftp", "email", "mail", "www"} do
			if x == h then
				return true
			end
		end
		return false
	end
	if is_not_name(hostname[1]) then
		name = hostname[2]
	else
		name = hostname[1]
	end

	local root, i = name, 2
	while servers[name] do
		name = root .. i
		i = i + 1
	end
	return name
end
