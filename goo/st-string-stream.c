/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/*
 * st-string-stream.c
 *
 * Copyright (C) 2008 Vincent Geddes <vgeddes@gnome.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <st-string-stream.h>

#include <string.h>

#include <stdbool.h>

#include <stdlib.h>

struct _GooStringStream
{
    gunichar *text;

    guint p;			/* current index into text */

    guint n;			/* total number of chars in text */
    guint line;			/* current line number, starting from 1 */
    guint column;		/* current column number, starting from 1 */

    GList *markers;
    guint last_marker;
    guint mark_depth;

};

typedef struct _Marker
{
    int p;
    int line;
    int column;

} Marker;


void
st_string_stream_destroy (GooStringStream * stream)
{
    GList *l;

    g_assert (stream != NULL);

    g_free (stream->text);

    for (l = stream->markers; l; l = l->next) {
	g_slice_free (Marker, l->data);
    }
    g_list_free (stream->markers);
}

guint
st_string_stream_get_line (GooStringStream * stream)
{
    g_assert (stream != NULL);

    return stream->line;
}

guint
st_string_stream_get_column (GooStringStream * stream)
{
    g_assert (stream != NULL);

    return stream->column;
}

gunichar
st_string_stream_look_ahead (GooStringStream * stream, int i)
{
    g_assert (stream != NULL);

    if (G_UNLIKELY (i == 0)) {
	return 0x0000;
    }

    if (G_UNLIKELY (i < 0)) {
	i++;

	if ((stream->p + i - 1) < 0) {
	    // invalid
	    return ST_STRING_STREAM_EOF;
	}

    }

    if ((stream->p + i - 1) >= stream->n) {
	return ST_STRING_STREAM_EOF;
    }

    return stream->text[stream->p + i - 1];
}

int
st_string_stream_mark (GooStringStream * stream)
{
    Marker *marker;

    g_assert (stream != NULL);

    stream->mark_depth++;

    if (stream->markers == NULL) {

	// depth 0 means no backtracking
	stream->markers = g_list_append (stream->markers, NULL);
    }

    if (stream->mark_depth >= g_list_length (stream->markers)) {
	marker = g_slice_new (Marker);
	stream->markers = g_list_append (stream->markers, marker);
    } else {
	marker = g_list_nth_data (stream->markers, stream->mark_depth);
    }

    marker->p = stream->p;
    marker->line = stream->line;
    marker->column = stream->column;

    stream->last_marker = stream->mark_depth;

    return stream->mark_depth;
}

void
st_string_stream_rewind_to_marker (GooStringStream * stream, guint m)
{
    g_assert (stream != NULL);

    Marker *marker = (Marker *) g_list_nth_data (stream->markers, m);

    st_string_stream_seek (stream, marker->p);

    stream->line = marker->line;
    stream->column = marker->column;

    // unwind any other markers made after m and release m  
    stream->mark_depth = m;

    // release this marker  
    stream->mark_depth--;
}


void
st_string_stream_rewind (GooStringStream * stream)
{
    st_string_stream_rewind_to_marker (stream, stream->last_marker);
}

void
st_string_stream_seek (GooStringStream * stream, guint index)
{
    g_assert (stream != NULL);

    if (index <= stream->p) {
	stream->p = index;
    }

    while (stream->p < index) {
	st_string_stream_consume (stream);
    }
}

void
st_string_stream_reset (GooStringStream * stream)
{
    g_assert (stream != NULL);

    stream->p = 0;
    stream->line = 1;
    stream->column = 1;
    stream->mark_depth = 0;
}


void
st_string_stream_consume (GooStringStream * stream)
{
    g_assert (stream != NULL);

    if (stream->p < stream->n) {

	stream->column++;

	if (stream->text[stream->p] == 0x000a) {
	    stream->line++;
	    stream->column = 1;
	}

	stream->p++;
    }
}

guint
st_string_stream_size (GooStringStream * stream)
{
    g_assert (stream != NULL);

    return stream->n;
}

#if 0
/* we don't use this for anything yet */
static gunichar *
st_string_stream_substring (GooStringStream * stream, guint start, guint end)
{
    g_assert (stream != NULL);
    g_assert (end >= start);
    g_assert (end <= stream->n);

    gunichar *buffer;

    buffer = g_malloc (sizeof (gunichar) * (end - start) + 1);

    memcpy (buffer, stream->text + start, sizeof (gunichar) * (end - start));

    // null terminate
    buffer[end - start] = 0x0000;

    return buffer;
}
#endif

char *
st_string_stream_substring (GooStringStream * stream, guint start, guint end)
{
    char *buf;
    GError *error = NULL;

    buf = g_ucs4_to_utf8 (stream->text + start, end - start, NULL, NULL, &error);



    if (!buf) {
	g_critical (error->message);
    }

    return buf;
}

guint
st_string_stream_index (GooStringStream * stream)
{
    g_assert (stream != NULL);

    return stream->p;
}

static void
set_data (GooStringStream * stream, const char *string)
{
    long len;
    gunichar *buffer;
    GError *error = NULL;

    buffer = g_utf8_to_ucs4 (string, -1, NULL, &len, &error);
    if (error) {
	g_warning (error->message);
	exit (1);

    }

    stream->text = buffer;
    stream->n = (guint) len;

    stream->line = 1;
    stream->column = 1;
}

GooStringStream *
st_string_stream_new (const char *string)
{
    g_assert (string != NULL);

    GooStringStream *stream = g_slice_new (GooStringStream);

    set_data (stream, string);

    stream->markers = NULL;
    stream->mark_depth = 0;
    stream->last_marker = 0;

    return stream;
}
