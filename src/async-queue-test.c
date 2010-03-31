/* Moonshine - a Lua-based chat client
 *
 * Copyright (C) 2010 Dylan William Hardison
 *
 * This file is part of Moonshine.
 *
 * Moonshine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Moonshine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Moonshine.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <moonshine/async-queue-source.h>
#include <glib.h>
#include <unistd.h>

static guint tag = 0;

void my_free_func(gpointer data)
{
    g_print("free: %s\n",  ((GString *)data)->str);
    g_string_free((GString *)data, TRUE);
}

void my_pool_func(gpointer data, gpointer userdata)
{
    static int count   = 1;
    GAsyncQueue *queue = userdata;
    GString *name      = data;
    GString *msg       = g_string_new("");
    g_string_printf(msg, "count: %d", count++);

    g_print("- %s\n", name->str);
    g_string_free(name, TRUE);

    g_print("queue: %s\n", msg->str);
    g_async_queue_push(queue, msg);
}

gboolean my_queue_func(gpointer data, gpointer userdata)
{
    GString *msg = data;
    GString *prefix = userdata;

    g_print("%s%s\n", prefix->str, msg->str);

    g_string_free(msg, TRUE);
    return TRUE;
}

gboolean my_start_func(gpointer userdata)
{
    GAsyncQueue *queue = g_async_queue_new_full(my_free_func);
    GThreadPool *pool = g_thread_pool_new(my_pool_func, g_async_queue_ref(queue), 1, TRUE, NULL);
    tag = ms_async_queue_add_watch(queue, my_queue_func, g_string_new("msg: "), my_free_func);

    g_async_queue_unref(queue);
    g_thread_pool_push(pool, g_string_new("foo"), NULL);
    g_thread_pool_push(pool, g_string_new("bar"), NULL);
    g_thread_pool_push(pool, g_string_new("baz"), NULL);

    return FALSE;
}

gboolean my_beep_func(gpointer userdata)
{
    g_print("beep!\n");
    return FALSE;
}

gboolean my_stop_func(gpointer userdata)
{
    g_print("stopping...\n");
    gboolean rv = g_source_remove(tag);
    g_print("g_source_remove(%d): %s\n", tag, rv ? "TRUE" : "FALSE");
    return FALSE;
}

int main(int argc, char *argv[])
{
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);

    g_thread_init(NULL);

    g_timeout_add(10, my_start_func, NULL);
    //g_timeout_add(1000, my_beep_func, NULL);
    //g_timeout_add(5000, my_beep_func, NULL);
    g_timeout_add(5000, my_stop_func, NULL);
    g_main_loop_run(loop);

    return 0;
}
