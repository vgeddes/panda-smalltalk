/*
 * st-input.c
 *
 * Copyright (C) 2008 Vincent Geddes
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
*/


#include "st-input.h"

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct Marker
{
    int p;
    int line;
    int column;

} Marker;

struct STInput
{
    gunichar *text;

    guint p;	    /* current index into text */

    guint n;	    /* total number of chars in text */
    guint line;     /* current line number, starting from 1 */
    guint column;   /* current column number, starting from 1 */

    Marker marker;
};

void
st_input_destroy (STInput *input)
{
    g_assert (input != NULL);

    g_free (input->text);
}

guint
st_input_get_line (STInput *input)
{
    g_assert (input != NULL);

    return input->line;
}

guint
st_input_get_column (STInput *input)
{
    g_assert (input != NULL);

    return input->column;
}

gunichar
st_input_look_ahead (STInput *input, int i)
{
    g_assert (input != NULL);

    if (i == 0) {
	return 0x0000; /* undefined */
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

void
st_input_mark (STInput *input)
{
    g_assert (input != NULL);
    
    input->marker.p      = input->p;
    input->marker.line   = input->line;
    input->marker.column = input->column;
}

void
st_input_rewind (STInput *input)
{
    g_assert (input != NULL);
    
    st_input_seek (input, input->marker.p);

    input->line   = input->marker.line;
    input->column = input->marker.column;
}

void
st_input_seek (STInput *input, guint index)
{
    g_assert (input != NULL);

    if (index <= input->p) {
	input->p = index;
    }

    while (input->p < index)
	st_input_consume (input);
}

void
st_input_consume (STInput *input)
{
    g_assert (input != NULL);

    if (input->p < input->n) {

	input->column++;

	/* 0x000A is newline */
	if (input->text[input->p] == 0x000A) {
	    input->line++;
	    input->column = 1;
	}

	input->p++;
    }
}

guint
st_input_size (STInput *input)
{
    g_assert (input != NULL);

    return input->n;
}

char *
st_input_range (STInput *input, guint start, guint end)
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
st_input_index (STInput *input)
{
    g_assert (input != NULL);

    return input->p;
}

static void
set_data (STInput *input, const char *string)
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

STInput *
st_input_new (const char *string)
{
    g_assert (string != NULL);

    STInput *input = g_slice_new0 (STInput);

    set_data (input, string);

    input->marker.p = 0;
    input->marker.line = 0;
    input->marker.column = 0;

    return input;
}
