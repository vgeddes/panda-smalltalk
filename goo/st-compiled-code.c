/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/*
 * st-compiled-code.c
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

#include <st-compiled-code.h>

/* CompiledMethod */

ST_DEFINE_VTABLE (st_compiled_method, st_heap_object_vtable ())


     static bool is_compiled_method (void)
{
    return true;
}

static void
st_compiled_method_vtable_init (st_vtable_t * table)
{
    table->is_compiled_method = is_compiled_method;
}


/* CompiledBlock */

ST_DEFINE_VTABLE (st_compiled_block, st_heap_object_vtable ())

     static bool is_compiled_block (void)
{
    return true;
}

static void
st_compiled_block_vtable_init (st_vtable_t * table)
{
    table->is_compiled_block = is_compiled_block;
}
