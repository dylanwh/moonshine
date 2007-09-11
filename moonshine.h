/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#ifndef __MOONSHINE_H__
#define __MOONSHINE_H__

#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>

#include <slang.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <glib.h>
#include <gnet.h>
#include "config.h"

#define UNUSED __attribute__((unused))
#define PURE   __attribute__((pure)) 
#define CONST   __attribute__((pure)) 

/* typedefs */
typedef lua_State    LuaState;
typedef luaL_reg     LuaLReg;

/* moon.c: lua functions */
LuaState *moon_new(void);
gboolean moon_call(LuaState *L, const char *name, const char *sig, ...);
void moon_class_create(LuaState *L, const char *class, const LuaLReg methods[], const LuaLReg meta[]);
#define moon_require(L, package) moon_call(L, "require", "s", package)
#ifdef EMBED_LUA
void moon_loader_init(LuaState *L);
#endif

/* util.c: utility functions */
PURE int unicode_charwidth(gunichar ch);

/* term.c: terminal routines. */
void term_init(void);
void term_resize(void);
void term_color_set(const char *name, const char *fg, const char *bg);
void term_color_use(const char *name);
int term_color_to_id(const char *name);
const char *term_color_to_utf8(const char *name);
gunichar term_getkey(void);
#define term_refresh    SLsmg_refresh
#define term_goto       SLsmg_gotorc
#define TERM_COLS       SLtt_Screen_Cols 
#define TERM_LINES      SLtt_Screen_Rows
#define term_erase_eol  SLsmg_erase_eol
#define term_write_gunichar SLsmg_write_char
#define term_write_chars(s) SLsmg_write_nchars(s, strlen(s))
#define term_input_pending SLang_input_pending

/* lua modules */
int luaopen_Entry(LuaState *L);
int luaopen_Buffer(LuaState *L);
int luaopen_Topic(LuaState *L);
int luaopen_app(LuaState *L);
int luaopen_Client(LuaState *L);

#endif
