--[[
    Moonshine - a Lua-based chat client
    
    Copyright (C) 2010 Dylan William Hardison
    
    This file is part of Moonshine.
    
    Moonshine is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    Moonshine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with Moonshine.  If not, see <http://www.gnu.org/licenses/>.
]]

modules = {
   'moonshine.idle',
   'moonshine.log.core',
   'moonshine.loop',
   'moonshine.net.client',
   'moonshine.parseopt.core',
   'moonshine.timer',
   'moonshine.tree',
   'moonshine.ui.buffer',
   'moonshine.ui.entry',
   'moonshine.ui.label',
   'moonshine.ui.term',
   'purple.core',
   'purple.conversations',
   'purple.conversation',
   'purple.accounts',
   'purple.account',
   'purple.plugins',
   'purple.plugin',
   'purple.roomlist',
   'purple.room',
}

print "#ifndef __MOONSHINE_PRELOAD_H__"
print "#define __MOONSHINE_PRELOAD_H__"
function cfunc(x) return "luaopen_" .. x:gsub("%.", "_") end

print ""

for _, name in ipairs(modules) do
    print(string.format("extern int %s(LuaState *L);", cfunc(name)))
end

print ""

print "static inline void MS_PRELOAD_ALL(LuaState *L)\n{"
print "    g_assert(L);"
for _, name in ipairs(modules) do
    print(string.format("\tms_lua_preload(L, \"%s\", %s);", name, cfunc(name)))
end
print "}"

print ""

print "#endif"
