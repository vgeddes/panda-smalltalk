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

typedef struct STInput STInput;

enum
{
    ST_INPUT_EOF = -1,
};


STInput *st_input_new          (const char *string);

gunichar   st_input_look_ahead   (STInput *input, int i);

guint      st_input_get_line     (STInput *input);

guint      st_input_get_column   (STInput *input);

void       st_input_mark         (STInput *input);

void       st_input_rewind       (STInput *input);

void       st_input_seek    (STInput *input, guint index);

void       st_input_consume (STInput *input);

guint      st_input_size    (STInput *input);

guint      st_input_index   (STInput *input);

char      *st_input_range (STInput *input, guint start, guint end);

void       st_input_destroy   (STInput *input);

#endif /* __ST_INPUT_H__ */
