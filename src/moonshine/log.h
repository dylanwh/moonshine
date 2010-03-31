/*
    Moonshine - a Lua-based chat client
    
    Copyright (C) 2010 Dylan William Hardison
    
    This file is part of Moonshine.
    
    Moonshine is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    Moonshine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with Moonshine.  If not, see <http://www.gnu.org/licenses/>.
*/

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
void ms_log_handler(const gchar *log_domain, GLogLevelFlags log_level, const char *message, gpointer user_data);
void ms_log_replay(MSLog *log, GLogFunc func, gpointer user_data);
void ms_log_free(MSLog *log);

#endif
