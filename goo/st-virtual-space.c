/*
 * st-virtual-space.c
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

#include "st-virtual-space.h"
#include "st-types.h"

#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <string.h>

// adjust reserved size to be a integral multiple of the page size
static guint
adjust_size (int reserved_size)
{
    int page_size = getpagesize ();

    if (reserved_size % page_size != 0) {
	return (reserved_size / page_size) * page_size + page_size;
    }

    return reserved_size;
}

static bool
map_size (STVirtualSpace *space, guint size)
{
    void *start;

    guint adjusted_size = adjust_size (size);

    start = mmap (NULL, adjusted_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

    if (((long) start) == -1) {
	g_debug (strerror (errno));
	return false;
    }
    
    space->start = (st_oop*) start;
    space->end = ((st_oop*) start) + adjusted_size / SIZEOF_VOID_P;
    
    return true;
}

STVirtualSpace *
st_virtual_space_new (void)
{
    return g_new (STVirtualSpace, 1);
}

bool
st_virtual_space_reserve (STVirtualSpace *space, guint size)
{
    return map_size (space, size);
}

void *
st_virtual_space_start (STVirtualSpace *space)
{
    return space->start;
}

void *
st_virtual_space_end (STVirtualSpace *space)
{
    return space->end;
}

void
st_virtual_space_destroy (STVirtualSpace *space)
{
    g_free (space);
}
