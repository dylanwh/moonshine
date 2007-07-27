/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#ifndef __MOONSHINE_H__
#define __MOONSHINE_H__

#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>

#include <event.h>
#include <slang.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <glib.h>

#define UNUSED __attribute__((unused))
#define PURE   __attribute__((pure)) 
#define THREAD __thread

/* typedefs */
typedef struct event Event;
typedef lua_State    LuaState;
typedef luaL_reg     LuaLReg;

/* util.c: utility functions */
gboolean moon_call(LuaState *L, const char *name, const char *sig, ...);
void moon_class_create(LuaState *L, const char *class, const LuaLReg methods[], const LuaLReg meta[]);

PURE int unicode_charwidth(gunichar ch);

/* term.c: terminal routines. */
void term_init(void);
void term_resize(void);
void term_reset(void);
gunichar term_getkey(void);

void term_color_set(const char *name, const char *fg, const char *bg);
void term_color_use(const char *name);
int term_color_to_id(const char *name);
const char *term_color_to_utf8(const char *name);

#define term_refresh    SLsmg_refresh
#define term_goto       SLsmg_gotorc
#define TERM_COLS       SLtt_Screen_Cols 
#define TERM_LINES      SLtt_Screen_Rows
#define term_erase_eol  SLsmg_erase_eol
#define term_write_char SLsmg_write_char

void modEntry_register (lua_State *L); // Provides the Entry class.
void modBuffer_register (lua_State *L); // Provides the Buffer class.
void modapp_register(LuaState *L, GMainLoop *loop);

#endif
