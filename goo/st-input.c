/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/*
 * st-string-input.c
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

#include <st-string-input.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

struct st_input_t
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

typedef struct marker_t
{
    int p;
    int line;
    int column;

} marker_t;


void
st_input_destroy (st_input_t *input)
{
    GList *l;

    g_assert (input != NULL);

    g_free (input->text);

    for (l = input->markers; l; l = l->next) {
	g_slice_free (marker_t, l->data);
    }
    g_list_free (input->markers);
}

guint
st_input_get_line (st_input_t *input)
{
    g_assert (input != NULL);

    return input->line;
}

guint
st_input_get_column (st_input_t *input)
{
    g_assert (input != NULL);

    return input->column;
}

gunichar
st_input_look_ahead (st_input_t *input, int i)
{
    g_assert (input != NULL);

    if (i == 0) {
	return 0x0000;
    }

    if (i < 0) {
	i++;

	if ((input->p + i - 1) < 0) {
	    // invalid
	    return ST_INPUT_EOF;
	}

    }

    if ((input->p + i - 1) >= input->n) {
	return ST_INPUT_EOF;
    }

    return input->text[input->p + i - 1];
}

int
st_input_mark (st_input_t *input)
{
    marker_t *marker;

    g_assert (input != NULL);

    input->mark_depth++;

    if (input->markers == NULL) {

	// depth 0 means no backtracking
	input->markers = g_list_append (input->markers, NULL);
    }

    if (input->mark_depth >= g_list_length (input->markers)) {
	marker = g_slice_new (marker_t);
	input->markers = g_list_append (input->markers, marker);
    } else {
	marker = g_list_nth_data (input->markers, input->mark_depth);
    }

    marker->p = input->p;
    marker->line = input->line;
    marker->column = input->column;

    input->last_marker = input->mark_depth;

    return input->mark_depth;
}

void
st_input_rewind_to_marker (st_input_t *input, guint m)
{
    g_assert (input != NULL);

    marker_t *marker = (marker_t *) g_list_nth_data (input->markers, m);

    st_input_seek (input, marker->p);

    input->line = marker->line;
    input->column = marker->column;

    // unwind any other markers made after m and release m  
    input->mark_depth = m;

    // release this marker  
    input->mark_depth--;
}


void
st_input_rewind (st_input_t *input)
{
    st_input_rewind_to_marker (input, input->last_marker);
}

void
st_input_seek (st_input_t *input, guint index)
{
    g_assert (input != NULL);

    if (index <= input->p) {
	input->p = index;
    }

    while (input->p < index) {
	st_input_consume (input);
    }
}

void
st_input_reset (st_input_t *input)
{
    g_assert (input != NULL);

    input->p = 0;
    input->line = 1;
    input->column = 1;
    input->mark_depth = 0;
}


void
st_input_consume (st_input_t *input)
{
    g_assert (input != NULL);

    if (input->p < input->n) {

	input->column++;

	if (input->text[input->p] == 0x000a) {
	    input->line++;
	    input->column = 1;
	}

	input->p++;
    }
}

guint
st_input_size (st_input_t *input)
{
    g_assert (input != NULL);

    return input->n;
}

#if 0
/* we don't use this for anything yet */
static gunichar *
st_input_substring (st_input_t *input, guint start, guint end)
{
    g_assert (input != NULL);
    g_assert (end >= start);
    g_assert (end <= input->n);

    gunichar *buffer;

    buffer = g_malloc (sizeof (gunichar) * (end - start) + 1);

    memcpy (buffer, input->text + start, sizeof (gunichar) * (end - start));

    // null terminate
    buffer[end - start] = 0x0000;

    return buffer;
}
#endif

char *
st_input_substring (st_input_t *input, guint start, guint end)
{
    char *buf;
    GError *error = NULL;

    buf = g_ucs4_to_utf8 (input->text + start, end - start, NULL, NULL, &error);



    if (!buf) {
	g_critical (error->message);
    }

    return buf;
}

guint
st_input_index (st_input_t *input)
{
    g_assert (input != NULL);

    return input->p;
}

static void
set_data (st_input_t *input, const char *string)
{
    long len;
    gunichar *buffer;
    GError *error = NULL;

    buffer = g_utf8_to_ucs4 (string, -1, NULL, &len, &error);
    if (error) {
	g_warning (error->message);
	exit (1);
    }

    input->text = buffer;
    input->n = (guint) len;

    input->line = 1;
    input->column = 1;
}

st_input_t *
st_input_new (const char *string)
{
    g_assert (string != NULL);

    st_input_t *input = g_slice_new (st_input_t);

    set_data (input, string);

    input->markers = NULL;
    input->mark_depth = 0;
    input->last_marker = 0;

    return input;
}
