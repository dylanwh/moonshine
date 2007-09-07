local _D = {}

function declare (name, initval)
    rawset(_G, name, initval)
    _D[name] = true
end

setmetatable(_G, {
    __newindex = function (t, n, v)
        if not _D[n] then
          	error("attempt to write to undeclared var. "..n, 2)
        else
          	rawset(t, n, v)  -- do the actual set
        end
    end,
    __index = function (_, n)
        if not _D[n] then
          	error("attempt to read undeclared var. "..n, 2)
        else
          	return nil
        end
    end,
})
