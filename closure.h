#ifndef _SPOON_CLOSURE_H
#define _SPOON_CLOSURE_H

#include <glib.h>

typedef struct {
	GFunc func;
	gpointer env;
	GDestroyNotify destroy;
	gint refcnt;
} Closure;

Closure *closure_new(GFunc func, gpointer env, GDestroyNotify destroy);

Closure *closure_ref(Closure *c);
void closure_unref(Closure *c);

void closure_call(Closure *c, gpointer arg);



#endif
