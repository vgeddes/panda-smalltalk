/*
 * st-string-stream.h
 *
 * Copyright (C) 2008 Vincent Geddes
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __ST_STRING_STREAM_H__
#define __ST_STRING_STREAM_H__

#include <glib.h>

G_BEGIN_DECLS

typedef struct _GooStringStream GooStringStream;

enum
{
	ST_STRING_STREAM_EOF = -1,
};


GooStringStream  *st_string_stream_new         (const char *string);


gunichar          st_string_stream_look_ahead (GooStringStream *stream, int i);


guint             st_string_stream_get_line    (GooStringStream *stream);

guint             st_string_stream_get_column  (GooStringStream *stream);

int               st_string_stream_mark        (GooStringStream *stream);

void              st_string_stream_rewind      (GooStringStream *stream);

void              st_string_stream_rewind_to_marker (GooStringStream *stream, guint m);

void              st_string_stream_seek        (GooStringStream *stream, guint index);

void              st_string_stream_consume     (GooStringStream *stream);

void              st_string_stream_reset       (GooStringStream *stream);

guint             st_string_stream_size        (GooStringStream *stream);

guint             st_string_stream_index       (GooStringStream *stream);

char             *st_string_stream_substring   (GooStringStream *stream, guint start, guint end);


void              st_string_stream_destroy     (GooStringStream *stream);

G_END_DECLS

#endif /* __ST_STRING_STREAM_H__ */

