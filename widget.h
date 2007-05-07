/* vim: set ft=objc: */
#ifndef _SPOON_WIDGET_H
#define _SPOON_WIDGET_H
#include <glib.h>
#include <objc/Object.h>

@interface SpoonUI: Object
{
	int objnum;
}

- draw;

@end

#endif
