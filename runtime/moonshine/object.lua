local Object = { }
local mt     = { }

function mt.__index(self, slot)
    local proto = rawget(self, '__proto')
    if proto then
        return proto[slot]
    end
end

function Object.new(proto, ...)
    local self = setmetatable( { __proto = proto }, mt)
   
    local init = self.__init
    if init then
        init(self, ...)
    end

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

function Object:get_slot(name)
    return self[ '!' .. name ]
end

function Object:set_slot(name, val)
    self['!' .. name] = val
end

function Object:def_accessor(name)
    local slot = '!' .. name
    self[name] = function(self, ...)
        if select('#', ...) == 0 then
            return self[slot]
        else
            self[slot] = ...
            return self[slot]
        end
    end
end

return Object
