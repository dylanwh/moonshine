-- maps between limited integers and strings.
local Map = new "moonshine.object"

function Map:__init(max)
    self._to_id   = {}
    self._to_name = {}
    self._max   = max
end

function Map:assign(name, id)
    if id == nil then
        id = self:next_id()
    end

    assert(name ~= nil, "name must not be nil")
    assert(id <= self._max, "cannot assign id larger than " .. self._max)

    self._to_id[name] = id
    self._to_name[id] = name
    return id
end

function Map:find(name)
    return self._to_id[name]
end

function Map:find_or_assign(name)
    return self:find(name) or self:assign(name)
end

function Map:next_id()
    return #self._to_name + 1
end

return Map
