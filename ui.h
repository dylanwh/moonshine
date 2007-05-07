/* vim: set ft=objc: */
#ifndef _SPOON_UI_H
#define _SPOON_UI_H
#include <glib.h>
#include <objc/Object.h>

@interface SpoonUI: Object
{
	GString *topic;
	GList *buffer;
	GString *entry;
}

- (void) draw;

/*
SpoonUI *spoon_ui_new(void);
void spoon_ui_free(SpoonUI *);
void spoon_ui_getkey(SpoonUI *);
void spoon_ui_draw(SpoonUI *);
*/
@end

#endif
