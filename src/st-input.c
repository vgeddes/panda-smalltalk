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
#include "st-utils.h"

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <wchar.h>
#include <glib.h>

typedef struct Marker
{
    int p;
    int line;
    int column;

} Marker;

struct STInput
{
    wchar_t *text;

    st_uint p;	    /* current index into text */

    st_uint n;	    /* total number of chars in text */
    st_uint line;     /* current line number, starting from 1 */
    st_uint column;   /* current column number, starting from 1 */

    Marker marker;
};

static wchar_t *
st_wcstrdup (const wchar_t *string)
{
    wchar_t *buf;
    st_uint    len;

    st_assert (string != NULL);

    len = wcslen (string);

    buf = st_malloc (sizeof (wchar_t) * (len + 1));
    wmemcpy (buf, string, len);
    buf[len] = 0;

    return buf;
}

static wchar_t *
filter_double_bangs (const wchar_t *chunk)
{
    st_uint size, i = 0, count = 0;
    const wchar_t *p = chunk;
    wchar_t *buf;

    size = wcslen (chunk);

    if (size < 2)
	return st_wcstrdup (chunk);

    /* count number of redundant bangs */
    while (p[0] && p[1]) {
	if (ST_UNLIKELY (p[0] == '!' && p[1] == '!'))
	    count++;
	p++;
    }
    
    buf = st_malloc (sizeof (wchar_t) * (size - count + 1));

    /* copy over text skipping over redundant bangs */
    p = chunk;
    while (*p) {
	if (*p == '!')
	    p++;
	buf[i++] = *p;
	p++;
    }
    buf[i] = 0;

    return buf;
}

wchar_t *
st_input_next_chunk (STInput *input)
{
    wchar_t *chunk_filtered, *chunk = NULL;
    st_uint start;

    start = st_input_index (input);

    while (st_input_look_ahead (input, 1) != ST_INPUT_EOF) {

	if (st_input_look_ahead (input, 1) != '!') {
	    st_input_consume (input);
	    continue;
	}
	
	/* skip past doubled bangs */
	if (st_input_look_ahead (input, 1) == '!'
	    && st_input_look_ahead (input, 2) == '!') {
	    st_input_consume (input);
	    st_input_consume (input);
	    continue;
	}

	chunk = st_input_range_ucs4 (input, start, st_input_index (input));	
	chunk_filtered = filter_double_bangs (chunk);
	st_input_consume (input);
	st_free (chunk);	

	return chunk_filtered;
    }

    return NULL;
}

void
st_input_destroy (STInput *input)
{
    st_assert (input != NULL);

    st_free (input->text);
    g_slice_free (STInput, input);
}

st_uint
st_input_get_line (STInput *input)
{
    st_assert (input != NULL);

    return input->line;
}

st_uint
st_input_get_column (STInput *input)
{
    st_assert (input != NULL);

    return input->column;
}

wchar_t
st_input_look_ahead (STInput *input, int i)
{
    st_assert (input != NULL);

    if (ST_UNLIKELY (i == 0)) {
	return 0x0000;
    }

    if (ST_UNLIKELY (i < 0)) {
	i++;
	if ((input->p + i - 1) < 0)
	    return ST_INPUT_EOF;
    }

    if ((input->p + i - 1) >= input->n) {
	return ST_INPUT_EOF;
    }

    return input->text[input->p + i - 1];
}

void
st_input_mark (STInput *input)
{
    st_assert (input != NULL);
    
    input->marker.p      = input->p;
    input->marker.line   = input->line;
    input->marker.column = input->column;
}

void
st_input_rewind (STInput *input)
{
    st_assert (input != NULL);
    
    st_input_seek (input, input->marker.p);

    input->line   = input->marker.line;
    input->column = input->marker.column;
}

void
st_input_seek (STInput *input, st_uint index)
{
    st_assert (input != NULL);

    if (index <= input->p) {
	input->p = index;
    }

    while (input->p < index)
	st_input_consume (input);
}

void
st_input_consume (STInput *input)
{
    st_assert (input != NULL);

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

st_uint
st_input_size (STInput *input)
{
    st_assert (input != NULL);

    return input->n;
}


char *
st_input_range (STInput *input, st_uint start, st_uint end)
{
    char *buf;
    GError *error = NULL;
    
    buf = g_ucs4_to_utf8 ((const gunichar *) input->text + start, end - start, NULL, NULL, &error);
    
    if (!buf) {
	g_critical (error->message);
    }

    return buf;
}

wchar_t *
st_input_range_ucs4 (STInput *input, st_uint start, st_uint end)
{
    wchar_t *buf;
    st_uint    len;

    st_assert ((end - start) >= 0);

    len = end - start;
    buf = st_malloc (sizeof (wchar_t) * (len + 1));
    wmemcpy (buf, input->text + start, len);
    buf[len] = 0;

    return buf;
}

st_uint
st_input_index (STInput *input)
{
    st_assert (input != NULL);

    return input->p;
}

static void
initialize_state (STInput *input, const wchar_t *string)
{
    input->text    = (wchar_t *) string;
    input->n       = wcslen (string);
    input->line    = 1;
    input->column  = 1;

    input->marker.p      = 0;
    input->marker.line   = 0;
    input->marker.column = 0;
}

STInput *
st_input_new (const char *string)
{
    wchar_t *string_ucs4;
    STInput *input;

    st_assert (string != NULL);

    string_ucs4 = (wchar_t *) st_utf8_to_ucs4 (string);
    if (string_ucs4 == NULL)
	return NULL;

    input = st_new0 (STInput);

    initialize_state (input, string_ucs4);

    return input;
}

STInput *
st_input_new_ucs4 (const wchar_t *string)
{
    STInput *input;

    st_assert (string != NULL);

    input = st_new0 (STInput);

    initialize_state (input, st_wcstrdup (string));    

    return input;
}

