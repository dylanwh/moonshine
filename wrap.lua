function line(n,x)
	if not x then x = 0 end
	local t = debug.getinfo(n)
	return "#line ".. ( t.currentline + x ) .. ' "' .. t.short_src .. '"\n'
end

local CLASS
local METHODS = {} 
local META    = {}
local BOXED = line(2) .. [[
static <CLASS> *to<CLASS> (LuaState *L, int index)
{
  	<CLASS> **b = lua_touserdata(L, index);
  	if (b == NULL) luaL_typerror(L, index, "<CLASS>");
  	return *b;
}

static <CLASS> *check<CLASS> (LuaState *L, int index)
{
  	luaL_checktype(L, index, LUA_TUSERDATA);
  	<CLASS> **b = luaL_checkudata(L, index, "<CLASS>");
  	if (b == NULL) luaL_typerror(L, index, "<CLASS>");
  	return *b;
}

static <CLASS> **push<CLASS> (LuaState *L)
{
  	<CLASS> **b = (<CLASS> **)lua_newuserdata(L, sizeof(<CLASS> *));
  	luaL_getmetatable(L, "<CLASS>");
  	lua_setmetatable(L, -2);
  	return b;
}
]]
local UNBOXED = line(2) .. [[
static <CLASS> *to<CLASS> (LuaState *L, int index)
{
  	<CLASS> *b = lua_touserdata(L, index);
  	if (b == NULL) luaL_typerror(L, index, "<CLASS>");
  	return b;
}

static <CLASS> *check<CLASS> (LuaState *L, int index)
{
  	luaL_checktype(L, index, LUA_TUSERDATA);
  	<CLASS> *b = luaL_checkudata(L, index, "<CLASS>");
  	if (b == NULL) luaL_typerror(L, index, "<CLASS>");
  	return b;
}

static <CLASS> *push<CLASS> (LuaState *L)
{
  	<CLASS> *b = lua_newuserdata(L, sizeof(<CLASS>));
  	luaL_getmetatable(L, "<CLASS>");
  	lua_setmetatable(L, -2);
  	return b;
}
]]


function include(s)
	print (line(3) .. [[#include "]]..s..[["]])
end

function class(name)
	CLASS = name
	return function (t)
		if type(t) == 'table' then
			print (line(3).."typedef struct {")
			for k, v in pairs(t) do
				print ("\t"..v..' '..k..";")
			end
			print ("} "..name..";")
			local s = UNBOXED:gsub("<CLASS>", CLASS)
			print(s)
		else
			t()
		end
	end
end

function boxed()
	assert(CLASS)
	local s = BOXED:gsub("<CLASS>", CLASS)
	print(line(3)..s)
end

function method(name)
	local l = line(3,1)
	return function(code)
		print(l.."static int "..CLASS.."_"..name.."(LuaState *L)\n{\n"..code.."}\n")
		table.insert(METHODS, name)
	end
end

default = function ()
	local s = [[
	char buff[32];
  	sprintf(buff, "%p", to<CLASS>(L, 1));
  	lua_pushfstring(L, "<CLASS> (%s)", buff);
  	return 1;
]]
  	s = s:gsub("<CLASS>", CLASS)
  	return s
end

function meta(name)
	local l = line(3,1)
	return function(code)
		if type(code) == 'function' then
			code = code()
		end
		print(l.."static int "..CLASS.."_"..name.."(LuaState *L)\n{\n"..code.."}\n")
		table.insert(META, name)
	end
end

local function export(table, var)
	print("static const LuaLReg "..CLASS.."_"..var.."[] = {");
	for _, method in ipairs(table) do
		print('\t{"' .. method .. '", ' .. CLASS .. "_" .. method .. "},")
	end
	print ("\t{0, 0}\n};")
end

function finish (code)
	print(line(2))
	assert (CLASS)
	export(METHODS, "methods")
	export(META, "meta")
	print ("int luaopen_"..CLASS.."(LuaState *L)\n{")
	print ('\tmoon_class_create(L, "'..CLASS..'", '..CLASS..'_methods, '..CLASS..'_meta);')
	if code then
		io.stdout:write(line(3,-2))
		io.stdout:write(code)
		print(line(2))
	end
	print ('\treturn 1;')
	print ("}")
end


print [[#include "moon.h"]]
