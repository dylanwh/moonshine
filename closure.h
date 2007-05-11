#ifndef _SPOON_CLOSURE_H
#define _SPOON_CLOSURE_H

#include <glib.h>

typedef struct {
	GFunc func;
	gpointer env;
	GDestroyNotify destroy;
	gint refcnt;
} Closure;

Closure *spoon_closure_new(GFunc func, gpointer env, GDestroyNotify destroy);

Closure *spoon_closure_ref(Closure *c);
void spoon_closure_unref(Closure *c);

void spoon_closure_call(Closure *c, gpointer arg);



#endif
