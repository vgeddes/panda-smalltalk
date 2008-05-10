/*
 * st-virtual-space.h
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

#ifndef __ST_VIRTUAL_SPACE__
#define __ST_VIRTUAL_SPACE__

#include <st-types.h>

#include <glib.h>
#include <stdbool.h>

typedef struct 
{
    st_oop *start;
    st_oop *end;
} STVirtualSpace;

STVirtualSpace  *st_virtual_space_new      (void);

bool             st_virtual_space_reserve (STVirtualSpace *space, guint size);

void            *st_virtual_space_start    (STVirtualSpace *space);

void            *st_virtual_space_end      (STVirtualSpace *space);

void             st_virtual_space_destroy  (STVirtualSpace *space);


#endif /* __ST_VIRTUAL_SPACE__ */
