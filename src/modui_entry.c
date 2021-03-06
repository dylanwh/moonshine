/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80: */

/* Moonshine - a Lua-based chat client
 *
 * Copyright (C) 2010 Bryan Donlan, Dylan William Hardison
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

#include <moonshine/term.h>
#include <moonshine/lua.h>
#include <string.h>

#define CLASS "moonshine.ui.entry"

typedef struct {
    gunichar *buffer;
    gsize bufsize; /* The total size of the buffer in gunichars. Is zero iff buffer is NULL. */
    gsize bufused; /* The length of the actual string in the buffer. */
    gsize view_off; /* The index of the first visible character. May be out of range; entry_render will correct such issues. */
    gsize curs_off; /* The index of the character the cursor is on. Must not be out of range (> bufused). */
    gboolean dirty; /* True if the entry has been modified since the last set(), clear(), or clear_dirty() */
} Entry;

static int entry_new(LuaState *L)
{
    Entry *e = ms_lua_newclass(L, CLASS, sizeof(Entry));
    e->buffer = NULL;
    e->bufsize = e->bufused = 0;
    e->view_off = e->curs_off = 0;
    e->dirty    = FALSE;
    return 1;
}

inline static int keypress(Entry *e, gunichar uc)
{
    if (!uc || !g_unichar_isdefined(uc))
        return 0; /* Filter invalid characters, hopefully. XXX: is this enough to deny the PUA? */
    if (e->bufused + 1 > e->bufsize) {
        /* If e->bufused * sizeof(e->buffer[0]) > 2**32, then there's a theoretical heap overflow on Really Big Systems.
         * Let's just clamp it to something nice and sane like 16kilochars
         */
        if (e->bufsize >= 16384) return 0;
        /* We assume most lines will be relatively small, so just increase by 128 each time */
        e->bufsize += 128;
        e->buffer = g_renew(gunichar, e->buffer, e->bufsize);
    }
    memmove(e->buffer + e->curs_off + 1, e->buffer + e->curs_off,
            sizeof(e->buffer[0]) * (e->bufused - e->curs_off));
    e->buffer[e->curs_off] = uc;
    e->bufused++;
    e->curs_off++;
    e->dirty = TRUE;
    return 0;
}

static int entry_keypress(LuaState *L)
{
    Entry *e        = ms_lua_checkclass(L, CLASS, 1);
    const char *key = luaL_checkstring(L, 2);
    gunichar uc = g_utf8_get_char(key);

    return keypress(e, uc);
}

static int entry_move(LuaState *L)
{
    Entry *e   = ms_lua_checkclass(L, CLASS, 1);
    int offset = luaL_checkinteger(L, 2);  

    if (offset == 0) return 0;
    
    gsize new_off = e->curs_off + offset;
    if ((new_off < e->curs_off) != (offset < 0)) {
        /* Integer overflow has occured. Move the the end or start. */
        if (offset < 0)
            new_off = 0;
        else
            new_off = e->bufused;
    }
    if (new_off > e->bufused)
        new_off = e->bufused;
    e->curs_off = new_off;
    return 0;
}

static int entry_move_to(LuaState *L)
{
    Entry *e     = ms_lua_checkclass(L, CLASS, 1);
    int absolute = luaL_checkinteger(L, 2);  

    if (!e->bufused) return 0;

    if (absolute >= 0) {
        e->curs_off = MIN(e->bufused, (guint)absolute);
    } else { /* -1 is after the last char, -2 on the last, etc... */
        /* Note: If absolute is too far negative, we'll get gsize wraparound here. */
        e->curs_off = e->bufused + absolute + 1;
        if (e->curs_off > e->bufused)
            e->curs_off = 0;
    }
    return 0;
}

static int entry_get(LuaState *L)
{
    Entry *e  = ms_lua_checkclass(L, CLASS, 1);
    if (e->bufused == 0) {
        lua_pushstring(L, "");
        return 1;
    } else {
        char *str = g_ucs4_to_utf8(e->buffer, e->bufused, NULL, NULL, NULL);
        lua_pushstring(L, str);
        g_free(str);
        return 1;
    }
}

static int entry_clear(LuaState *L)
{
    Entry *e  = ms_lua_checkclass(L, CLASS, 1);
    e->bufused = 0;
    e->curs_off = e->view_off = 0;
    /* If we have more than a page of buffer, free it, to prevent a single
     * zomg-large string from using memory forever.
     */
    if (e->bufused > 1024) {
        g_free(e->buffer);
        e->bufused = 0;
    }
    e->dirty = FALSE;
    return 0;
}

static int entry_set(LuaState *L)
{
    Entry *e         = ms_lua_checkclass(L, CLASS, 1);
    const char *line = luaL_checkstring(L, 2);

    e->dirty = TRUE; // was 0 - why?
    GError *error;
    glong written;
    gunichar *buffer = g_utf8_to_ucs4(line, -1, NULL, &written, &error);
    if  (buffer) {
        g_free(e->buffer);

        e->bufsize = e->bufused = written;
        e->curs_off = e->bufused;
        e->view_off = 0;
        e->buffer = buffer;
        return 0;
    } else {
        lua_pushfstring(L, "Entry:set() - UCS4 conversion failed: %s", error->message);
        g_error_free(error);
        return lua_error(L);
    }
}

static guint center_view(Entry *e, guint width) {
    /* To find a new view window, we start at the cursor, and move out, trying
     * to keep the width used on each side roughly equal. As such, we keep a
     * tally of the width of each side, and just advance a character on the
     * lesser side. If we run out of characters on the left, obviously return 0
     * and left-justify the entire line on screen. However, if we run out on the
     * right, then pad out the right with the equivalent width of remaining chars
     * on the left, to center properly.
     */

    /* To simplify things, we ensure that there /is/ something on the left */
    if (e->curs_off == 0)
        return 0;

    /* To be precise, right_i starts at the cursor position itself */
    guint left_w = 0, right_w = 0;
    guint left_i = e->curs_off - 1;
    guint right_i = e->curs_off;

    while (left_w + right_w < width) {
        const guint total_width = left_w + right_w;
        /* XXX: This next if may break on terminals with less than 6 chars
         * for the input line, I think. */
        if (left_i == 0)
            return 0;

        if (left_w < right_w || right_i == e->bufused) {
            gunichar ch = e->buffer[left_i];
            int cwidth = ms_term_charwidth(ch);
            if (cwidth + total_width > width)
                break;
            left_i--;
            left_w += cwidth;
            if (right_i == e->bufused)
                right_w += cwidth;
        } else {
            gunichar ch = e->buffer[right_i];
            int cwidth = ms_term_charwidth(ch);
            if (cwidth + total_width > width)
                break;
            right_i++;
            right_w += cwidth;
        }
    }
    return left_i;
}

static int try_render(Entry *e, guint lmargin) {
    guint idx = e->view_off;
    guint width = 0;
    int curs_pos = -1;
    const guint max_width = MS_TERM_COLS - lmargin;

    ms_term_goto(MS_TERM_LINES - 1, lmargin);
    ms_term_erase_eol();

    if (e->view_off >= e->bufused && e->view_off != 0)
        return -1;

    while (idx < e->bufused) {
        if (idx == e->curs_off)
            curs_pos = width + lmargin;
        gunichar ch = e->buffer[idx];
        guint charwidth = ms_term_charwidth(ch);
        if (charwidth + width > max_width)
            break;
        width += charwidth;
        ms_term_write_gunichar(ch);
        idx++;
    }

    if (idx == e->bufused && e->curs_off == e->bufused && width < max_width)
        curs_pos = width + lmargin;

    if (curs_pos != -1)
        ms_term_goto(MS_TERM_LINES - 1, curs_pos);
    return curs_pos;
}

static int entry_render(LuaState *L)
{
    Entry *e           = ms_lua_checkclass(L, CLASS, 1);
    const char *prompt = luaL_checkstring(L, 2);

    g_assert(e->curs_off <= e->bufused);

    /* FIXME: This assumes 1 byte == 1 char */
    guint lmargin = strlen(prompt);
    ms_term_goto(MS_TERM_LINES - 1, 0);
    ms_term_write_chars((gchar *)prompt);

    if (try_render(e, lmargin) == -1) {
        e->view_off = center_view(e, MS_TERM_COLS - lmargin);
        if (try_render(e, lmargin) == -1) {
            /* This should never happen, but just in case. */
            if (e->curs_off == 0) {
                e->view_off = 0;
            } else {
                e->view_off = e->curs_off - 1;
            }
            try_render(e, lmargin);
        }
    }
    return 0;
}


static void erase_region(Entry *e, int start, int end)
{
    g_assert(start >= 0);
    g_assert(end   >= 0);
    g_assert(start <= end);
    g_assert((guint)start <= e->bufused);
    g_assert((guint)end   <= e->bufused);

    if (start == end)
        return;
    e->dirty = TRUE;
    memmove(e->buffer + start, e->buffer + end,
            sizeof(e->buffer[0]) * (e->bufused - end));
    e->bufused -= (end - start);

    if (e->curs_off > (guint)end)
        e->curs_off -= (end - start);
    else if (e->curs_off > (guint)start) /* in-between */
        e->curs_off = start;
    return;
}

static int entry_erase(LuaState *L)
{
    Entry *e  = ms_lua_checkclass(L, CLASS, 1);
    int count = luaL_checkinteger(L, 2);
    if (!count)
        return 0;

    if (count > 0) {
        /* Delete chars after the current cursor location. */
        int start = e->curs_off;
        int end   = MIN(e->bufused, e->curs_off + count);
        erase_region(e, start, end);
    } else {
        /* Delete chars before the current cursor location */
        guint start = e->curs_off + count;
        guint end   = e->curs_off;

        /* Note: if (-count) > e->curs_off, start will overflow and
         * be > e->bufused */
        if (start > e->bufused)
            start = 0;
        erase_region(e, start, end);
    }
    return 0;
}

static int entry_gc(LuaState *L)
{
    Entry *e = ms_lua_toclass(L, CLASS, 1);
    g_free(e->buffer); /* XXX: is glib's free safe with NULL? */
    return 0;
}

static int entry_tostring(LuaState *L)
{
    char buff[32];
    sprintf(buff, "%p", ms_lua_toclass(L, CLASS, 1));
    lua_pushfstring(L, "Entry (%s)", buff);
    return 1;
}

static inline int wordlen_dir(Entry *e, int direction) {
    int count = 0;
    int i = e->curs_off;
    gboolean expect_space = 1;
    g_assert(direction);
    
    if (direction == -1)
        i--;

    for (; i >= 0 && (guint)i < e->bufused; i += direction) {
        if (expect_space) {
            expect_space = g_unichar_isspace(e->buffer[i]);
        } else {
            if (g_unichar_isspace(e->buffer[i]))
                break;
        }
        count++;
    }
    return count;
}

static int entry_wordlen(LuaState *L)
{
    Entry *e  = ms_lua_checkclass(L, CLASS, 1);

    lua_pushinteger(L, wordlen_dir(e, -1));
    lua_pushinteger(L, wordlen_dir(e, 1));
    return 2;
}

static int entry_clear_dirty(LuaState *L)
{
    Entry *e  = ms_lua_checkclass(L, CLASS, 1);

    e->dirty = FALSE;
    return 0;
}

static int entry_is_dirty(LuaState *L)
{
    Entry *e  = ms_lua_checkclass(L, CLASS, 1);
    
    lua_pushboolean(L, e->dirty);
    return 1;
}

static const LuaLReg entry_methods[] = {
    {"new", entry_new},
    {"keypress", entry_keypress},
    {"move", entry_move},
    {"move_to", entry_move_to},
    {"get", entry_get},
    {"set", entry_set},
    {"clear", entry_clear},
    {"erase", entry_erase},
    {"render", entry_render},
    {"wordlen", entry_wordlen},
    {"clear_dirty", entry_clear_dirty},
    {"is_dirty", entry_is_dirty},
    {0, 0}
};

static const LuaLReg entry_meta[] = {
    {"__gc", entry_gc},
    {"__tostring", entry_tostring},
    {0, 0}
};

int luaopen_moonshine_ui_entry(LuaState *L)
{
    ms_lua_class_register(L, CLASS, entry_methods, entry_meta);
    return 1;
}
