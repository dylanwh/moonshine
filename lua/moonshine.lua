require "bind" 
require "ui"
require "cmd"
require "config"

function boot_hook()
	ui.render()
end

function quit_hook()
	ui.print("Shutdown: %1", "bob")
end

function string.join(delimiter, list)
  	local len = getn(list)
  	if len == 0 then 
    	return "" 
  	end
  	local string = list[1]
  	for i = 2, len do 
    	string = string .. delimiter .. list[i] 
  	end
  	return string
end

-- Split text into a list consisting of the strings in text,
-- separated by strings matching delimiter (which may be a pattern). 
-- example: strsplit(",%s*", "Anna, Bob, Charlie,Dolores")
function string.split(delimiter, text)
  	local list = {}
  	local pos = 1
  	if strfind("", delimiter, 1) then -- this would result in endless loops
    	error("delimiter matches empty string!")
  	end
  	while 1 do
    	local first, last = strfind(text, delimiter, pos)
    	if first then -- found?
      		tinsert(list, strsub(text, pos, first-1))
      		pos = last+1
    	else
      		tinsert(list, strsub(text, pos))
      		break
    	end
  	end
  	return list
end

function print(...)
	ui.print("%1", string.join("", arg))
end

