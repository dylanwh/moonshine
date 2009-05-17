#ifndef __MOONSHINE_LOG_H__
#define __MOONSHINE_LOG_H__
#include "moonshine/config.h"
#include <glib.h>

typedef struct {
	gchar *log_domain;
	GLogLevelFlags log_level;
	gchar *message;
} MSLogItem;

typedef struct {
	GSequence *buffer;
} MSLog;

MSLog *ms_log_new(void);
void ms_log_install(MSLog *log);
void ms_log_unwind(MSLog *log);
void ms_log_free(MSLog *log);

#endif
