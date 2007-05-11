#include "closure.h"

Closure *spoon_closure_new(GFunc func, gpointer env, GDestroyNotify destroy)
{
	Closure *c = g_new(Closure, 1);
	c->func = func;
	c->env = env;
	c->destroy = destroy;
	c->refcnt = 1;
	return c;
}

Closure *spoon_closure_ref(Closure *c)
{ 
	g_return_val_if_fail (c != NULL, NULL);
	g_return_val_if_fail (c->refcnt > 0, c);

	g_atomic_int_add (&c->refcnt, 1);
	return c;
}

static void spoon_closure_free(Closure *c)
{
	if (c->destroy)
		c->destroy(c->env);
	g_free(c);
	g_print("freed closure\n");
}

void spoon_closure_unref(Closure *c)
{
  	g_return_if_fail (c != NULL);
  	g_return_if_fail (c->refcnt > 0);

  	if (g_atomic_int_exchange_and_add (&c->refcnt, -1) - 1 == 0)
  		spoon_closure_free(c);
}

void spoon_closure_call(Closure *c, gpointer arg)
{
	c->func(c->env, arg);
}
