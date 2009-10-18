local Object = { }
local mt     = { }

function mt.__index(self, slot)
    local proto = rawget(self, '__proto')
    if proto then
        return proto[slot]
    end
end

function Object.new(proto, attr)
    local self = setmetatable( { __proto = proto }, mt)
   
    if attr then
        for k, v in pairs(attr) do
            local f = self[k]
            if f then
                f(self, v)
            end
        end
    end

    local init = self.__init
    if init then
        init(self)
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




return Object
