function string.join(delimiter, list)
  	local len = #list
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
  	if string.find("", delimiter, 1) then -- this would result in endless loops
    	error("delimiter matches empty string!")
  	end
  	while 1 do
    	local first, last = string.find(text, delimiter, pos)
    	if first then -- found?
      		table.insert(list, string.sub(text, pos, first-1))
      		pos = last+1
    	else
      		table.insert(list, string.sub(text, pos))
      		break
    	end
  	end
  	return list
end

function print(...)
	ui.print("%1", string.join("", arg))
end
