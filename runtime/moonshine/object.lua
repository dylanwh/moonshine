local Object = { }
local mt     = { }

function mt.__index(self, slot)
    local parent = rawget(self, '__parent')
    if parent then
        return parent[slot]
    end
end

function Object.new(parent, ...)
    local self = setmetatable( { __parent = parent }, mt)

    local init = self.__init
    if init then init(self, ...) end

    return self
end

function Object:callback(name, ...)
    local cb_args = { ... }

    return function(...)
        local args = { ... }
        for i, arg in ipairs(cb_args) do
            table.insert(args, i, arg)
        end
        return self[name](self, unpack(args))
    end
end

return Object
