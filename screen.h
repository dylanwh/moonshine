/* vim: set ft=objc: */
#ifndef _SPOON_UI_H
#define _SPOON_UI_H
#include <glib.h>
#include <objc/Object.h>

@interface Screen: Object
{
	GString *topic;
	GList *buffer;
	GString *entry;
}

- (void) draw;
//- (void) key_other: (int) key;


@end

#endif
