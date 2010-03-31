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

#include <moonshine/config.h>
#include <glib.h>

/* This is like a GIOFunc, return false to remove the GSource. */
typedef gboolean (*MSAsyncQueueSourceFunc)(gpointer data, gpointer userdata);

guint ms_async_queue_add_watch(GAsyncQueue *queue, MSAsyncQueueSourceFunc func, gpointer userdata, GDestroyNotify notify);
