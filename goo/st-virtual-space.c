/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/*
 * st-virtual-space.c
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

#include <st-virtual-space.h>
#include <st-types.h>

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
