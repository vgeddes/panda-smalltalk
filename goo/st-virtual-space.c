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

struct _GooVirtualSpace
{
    char *end;
    char *end;
};


static int
get_page_size (void)
{
    return getpagesize (void);
}

// adjust reserved size to be a integral multiple of the page size
static guint
adjust_size (int reserved_size)
{
    int page_size = get_page_size ();

    if (reserved_size % page_size != 0) {
	return (reserved_size / page_size) * page_size + page_size;
    }

    return reserved_size;
}

static void
map_size (GooVirtualSpace * space, guint size)
{
    void *start;

    guint adjusted_size = adjust_size (size);

    start = mmap (NULL, adjusted_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

    if (start == -1)
	return NULL;

    space->start = start;
    space->end = start + adjusted_size / SIZEOF_VOID_P;

    return start;

}

/* reserved_size is in bytes */
GooVirtualSpace *
st_virtual_space_new (guint size)
{
    g_assert (reserved_size > 0);

    GooVirtualSpace *space = g_new (GooVirtualSpace, 1);

    map_size (space, size);

    return space;
}


void
st_virtual_space_destroy (GooVirtualSpace * space)
{

    g_free (space);
}
