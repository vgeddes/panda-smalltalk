/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/*
 * st-string-stream.h
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

#ifndef __ST_STRING_STREAM_H__
#define __ST_STRING_STREAM_H__

#include <glib.h>

G_BEGIN_DECLS typedef struct _GooStringStream GooStringStream;

enum
{
    ST_STRING_STREAM_EOF = -1,
};


GooStringStream *st_string_stream_new (const char *string);


gunichar st_string_stream_look_ahead (GooStringStream * stream, int i);


guint st_string_stream_get_line (GooStringStream * stream);

guint st_string_stream_get_column (GooStringStream * stream);

int st_string_stream_mark (GooStringStream * stream);

void st_string_stream_rewind (GooStringStream * stream);

void st_string_stream_rewind_to_marker (GooStringStream * stream, guint m);

void st_string_stream_seek (GooStringStream * stream, guint index);

void st_string_stream_consume (GooStringStream * stream);

void st_string_stream_reset (GooStringStream * stream);

guint st_string_stream_size (GooStringStream * stream);

guint st_string_stream_index (GooStringStream * stream);

char *st_string_stream_substring (GooStringStream * stream, guint start, guint end);


void st_string_stream_destroy (GooStringStream * stream);

G_END_DECLS
#endif /* __ST_STRING_STREAM_H__ */
