servers = {}

function server_tag(hostname)
	local tag
	local hostname = split(".", hostname)
	local function is_not_tag(h)
		for i, x in ipairs {"chat", "haver", "irc", "web", "ftp", "email", "mail", "www"} do
			if x == h then
				return true
			end
		end
		return false
	end
	if is_not_tag(hostname[1]) then
		tag = hostname[2]
	else
		tag = hostname[1]
	end

	local root, i = tag, 2
	while servers[tag] do
		tag = root .. i
		i = i + 1
	end
	return tag
end
