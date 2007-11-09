function join(sep, list)
	return table.concat(list, sep)
end

function split(div,str)
    if (div=='') then return false end
    local pos,arr = 0,{}
    local function iter()
    	return string.find(str,div,pos,true)
    end 
    -- for each divider found
    for st, sp in iter do
    	table.insert(arr,string.sub(str,pos,st-1)) -- Attach chars left of current divider
    	pos = sp + 1 -- Jump past current divider
	end
	table.insert(arr,string.sub(str,pos)) -- Attach chars right of last divider
	return arr
end


