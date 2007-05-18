#ifndef __SPOON_MOON_H__
#define __SPOON_MOON_H__
#include <glib.h>
#include "closure.h"

typedef struct _Moon Moon;

Moon *moon_new(void);
void moon_bind(Moon *M, const char *name, Closure *c);
void moon_call(Moon *M, const char *name, gpointer arg);

#endif
