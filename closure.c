#include "closure.h"

Closure *spoon_closure_new(GFunc func, gpointer env, GDestroyNotify destroy)
{
	Closure *c = g_new(Closure, 1);
	c->func = func;
	c->env = env;
	c->destroy = destroy;
	return c;
}

void spoon_closure_free(Closure *c)
{
	if (c->destroy)
		c->destroy(env);
	g_free(c);
}

void *spoon_closure_call(Closure *c, gpointer arg)
{
	c->func(c->env, arg);
}
