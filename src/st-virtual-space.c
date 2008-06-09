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
#include "st-utils.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>
#include <string.h>

// adjust reserved size to be a integral multiple of the page size
static st_uint
adjust_size (int reserved_size)
{
    int page_size = getpagesize ();

    if (reserved_size % page_size != 0) {
	return (reserved_size / page_size) * page_size + page_size;
    }

    return reserved_size;
}

static bool
map_size (st_virtual_space *space, st_uint size)
{
    void *start;

    st_uint adjusted_size = adjust_size (size);

    start = mmap (NULL, adjusted_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

    if (((long) start) == -1) {
	fprintf (stderr, strerror (errno));
	return false;
    }
    
    space->start = (st_oop*) start;
    space->end = ((st_oop*) start) + adjusted_size / SIZEOF_VOID_P;
    
    return true;
}

st_virtual_space *
st_virtual_space_new (void)
{
    return st_new0 (st_virtual_space);
}

bool
st_virtual_space_reserve (st_virtual_space *space, st_uint size)
{
    return map_size (space, size);
}

void *
st_virtual_space_start (st_virtual_space *space)
{
    return space->start;
}

void *
st_virtual_space_end (st_virtual_space *space)
{
    return space->end;
}

void
st_virtual_space_destroy (st_virtual_space *space)
{
    st_free (space);
}
