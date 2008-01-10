/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/*
 * st-utils.c
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

#include <st-utils.h>
#include <st-array.h>
#include <st-byte-array.h>

GList *objects = NULL;

st_oop_t
st_allocate_object (gsize size)
{
    // all heap objects have at least a header of two st_oop_ts
    g_assert (size >= 2);
    /* no gc fanciness yet */

    st_oop_t object = ST_OOP (g_malloc (size * sizeof (st_oop_t)));
    objects = g_list_append (objects, object);

    return object;
}
