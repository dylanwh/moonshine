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
   'moonshine.ui.statusbar',
   'moonshine.ui.term',
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
