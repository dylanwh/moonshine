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
LuaState *moon_init(void);
gboolean moon_call(lua_State *L, const char *name, const char *sig, ...);
PURE int unicode_charwidth(gunichar ch);

/* term.c: terminal routines. */
void term_init(void);
void term_resize(void);
void term_reset(void);
gunichar term_getkey(void);
#define term_refresh    SLsmg_refresh
#define term_goto       SLsmg_gotorc
#define TERM_COLS       SLtt_Screen_Cols 
#define TERM_LINES      SLtt_Screen_Rows
#define term_erase_eol  SLsmg_erase_eol
#define term_write_char SLsmg_write_char

/* screen.c: high-level terminal routines. */
//typedef struct Screen Screen;
//Screen *screen_new(void);
//void screen_refresh(Screen *scr);

void modEntry_register (lua_State *L); // Provides the Entry class.
void modapp_register(LuaState *L);

#endif
