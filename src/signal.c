/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80: */

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

#include "moonshine/config.h"
#include "moonshine/signal.h"

#include <unistd.h>
#include <stdio.h>
#include <glib.h>
#include <signal.h>

static GIOChannel *input_chan;
static int output_fd;
static GHashTable *signals;
static guint tag;

typedef struct {
    MSSignalFunc func;
    gpointer data;
    GDestroyNotify destroy;
} MSSignalCallback;

static gboolean on_input(GIOChannel *chan, UNUSED GIOCondition c, UNUSED gpointer p)/*{{{*/
{
    int       sig    = 0;
    gsize     bytes  = 0;
    GIOStatus status = 0;

    status = g_io_channel_read_chars(chan, (char *)&sig, sizeof(sig), &bytes, NULL);
    if (status == G_IO_STATUS_NORMAL) {
        MSSignalCallback *cb = g_hash_table_lookup(signals, GINT_TO_POINTER(sig));
        if (cb) cb->func(sig, cb->data);
    } 
    return TRUE;
}/*}}}*/

static void on_signal(int sig)/*{{{*/
{
    write(output_fd, &sig, sizeof(sig));
}/*}}}*/

static GIOChannel *channel_from_fd(int fd)/*{{{*/
{
    GIOChannel *channel = g_io_channel_unix_new(fd);
    g_io_channel_set_encoding(channel, NULL, NULL);
    g_io_channel_set_buffered(channel, FALSE);
    g_io_channel_set_flags(channel, G_IO_FLAG_NONBLOCK, NULL);
    g_io_channel_set_close_on_unref(channel, TRUE);
    return channel;
}/*}}}*/

void ms_signal_init(void)/*{{{*/
{
    int fildes[2];
    if (pipe(fildes) != 0) perror("pipe");

    signal(SIGCHLD, SIG_IGN); // libpurple needs this.
    input_chan = channel_from_fd(fildes[0]);
    output_fd  = fildes[1];
    signals    = g_hash_table_new(NULL, NULL);
    tag        = g_io_add_watch(input_chan, G_IO_IN, on_input, NULL);
}/*}}}*/

static void clear_each(gpointer key, UNUSED gpointer value, UNUSED gpointer data)/*{{{*/
{
    ms_signal_clear(GPOINTER_TO_INT(key));
}/*}}}*/

void ms_signal_reset(void)/*{{{*/
{
    g_hash_table_foreach(signals, clear_each, NULL);

    g_source_remove(tag);
    g_io_channel_unref(input_chan);
    close(output_fd);
    output_fd = 0;

    if (signals != NULL) {
        g_hash_table_destroy(signals);
    }
}/*}}}*/

void ms_signal_catch(int sig, MSSignalFunc func, gpointer data, GDestroyNotify destroy)/*{{{*/
{
    /* Set up the structure to specify the new action. */
    struct sigaction action;
    action.sa_handler = on_signal;
    sigemptyset (&action.sa_mask);
    action.sa_flags = 0;

    MSSignalCallback *callback = g_new(MSSignalCallback, 1);
    callback->func = func;
    callback->data = data;
    callback->destroy = destroy;
    g_hash_table_insert(signals, GINT_TO_POINTER(sig), callback);

    sigaction (sig, &action, NULL);
}/*}}}*/

void ms_signal_clear(int sig)/*{{{*/
{
    MSSignalCallback *cb = g_hash_table_lookup(signals, GINT_TO_POINTER(sig));
    struct sigaction action;
    action.sa_handler = SIG_DFL;
    sigemptyset (&action.sa_mask);
    action.sa_flags = 0;

    g_return_if_fail(cb != NULL);
    
    sigaction(sig, &action, NULL);
    g_hash_table_remove(signals, GINT_TO_POINTER(sig));

    if (cb->data && cb->destroy)
        cb->destroy(cb->data);
}/*}}}*/
