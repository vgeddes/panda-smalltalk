/*
 * st-input.h
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

#ifndef __ST_INPUT_H__
#define __ST_INPUT_H__

#include <glib.h>

typedef struct st_input_t st_input_t;

enum
{
    ST_INPUT_EOF = -1,
};


st_input_t *st_input_new          (const char *string);

gunichar   st_input_look_ahead   (st_input_t *input, int i);

guint      st_input_get_line     (st_input_t *input);

guint      st_input_get_column   (st_input_t *input);

void       st_input_mark         (st_input_t *input);

void       st_input_rewind       (st_input_t *input);

void       st_input_seek    (st_input_t *input, guint index);

void       st_input_consume (st_input_t *input);

guint      st_input_size    (st_input_t *input);

guint      st_input_index   (st_input_t *input);

char      *st_input_range (st_input_t *input, guint start, guint end);

void       st_input_destroy   (st_input_t *input);

#endif /* __ST_INPUT_H__ */
