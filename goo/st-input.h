/*
 * st-input.h
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
