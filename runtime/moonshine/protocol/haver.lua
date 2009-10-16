local Protocol = require "moonshine.protocol"
local Haver = Protocol:clone { __type   = 'Haver' }

function Haver:on_connect(fd, ...)
    Protocol.on_connect(self, fd, ...)
    if fd then
        self:send("HAVER", "Moonshine/"..VERSION)
    end
end

function Haver:on_readline(line)
    local msg = split("\t", line:gsub("\r$", ""))
    local cmd = table.remove(msg, 1):upper()
    if self[cmd] then
        self[cmd](self, unpack(msg))
    else
        screen:debug("Unknown command: %1 (%2)", cmd, join(", ", msg))
    end
end

function Haver:send(...)
    if self.handle then
        self.handle:write( join("\t", {...}) .. "\n" )
    end
end

function Haver:join(room) self:send('JOIN', room) end
function Haver:part(room) self:send('PART', room) end

function Haver:msg(target, kind, msg)
    local cmd
    if target.type == 'room' then
        cmd = 'IN'
        public_message_sent_hook(self, target.name, self.username, kind, msg)
    elseif target.type == 'user' then
        cmd = 'TO'
        private_message_sent_hook(self, target.name, kind, msg)
    else
        screen:debug("Unknown target type: %1", target.type)
    end
    self:send(cmd, target.name, kind, msg)
end

function Haver:usersof(room)
    self:send('USERSOF', room)
end

function Haver:HAVER(host, version, extensions)
    self.server_version = version
    self.extensions = split(",", extensions)
    
    screen:debug("trying to log in as %1", self.username)
    self:send('IDENT', self.username)
end

function Haver:HELLO(username, address)
    screen:debug("Logged in as %1", username)
end

function Haver:IN(room, user, type, msg)
    if not (self.username:lower() == user:lower()) then
        public_message_hook(self, room, user, type, msg)
    end
end

function Haver:FROM(user, type, msg)
    private_message_hook(self, user, type, msg)
end

function Haver:JOIN(room, user)
    join_hook(self, room, user)
end

function Haver:PART(room, user, ...)
    part_hook(self, room, user)
end

function Haver:PING(token)
    self:send('PONG', token)
end

function Haver:USERSOF(room, ...)
    local users = {...}
    userlist_hook(self, room, users)
end

return Haver
