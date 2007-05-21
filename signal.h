/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#ifndef __SPOON_SIGNAL_H__
#define __SPOON_SIGNAL_H__

#include <glib.h>
#include <lua.h>

#include <signal.h>
#include <unistd.h>

void signal_init(lua_State *L);
void signal_reset(void);

#define signal_catch(sig) signal_catch_full(sig, #sig)
void signal_catch_full(int sig, const char *name);

#endif
