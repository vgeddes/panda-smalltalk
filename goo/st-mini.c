/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/*
 * st-mini.c
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

/*
 * Mini is a lightweight virtual dispatch mechanism inspired by GObject. We use
 * it so that the VM code can work with st_oop_t's in an object-orientated manner,
 * despite the fact that the VM is written in C. Mini is only used as a design
 * convenience in VM layer.
 *
 * Basically, Mini allows one to create a table of function pointers, set up in such a way
 * as to provide polymorphic behaviour. Each new table inherits the behaviour
 * of a parent table.
 *
 * Mini is lightweight because it has no notion of instances and instance variables,
 * only providing a skeleton for implementing polymorphic methods. This makes it
 * ideal for embedding in existing object systems (such as Smalltalk st_oop_ts).
 *
 * We embed a Mini table in each Class st_oop_t. All oop's then delegate
 * any polymorphic functionality to their class st_oop_t (where a mini table is embedded).
 *
 */

#include "st-mini.h"

#include "string.h"
#include "stdbool.h"

#include <glib.h>

#define NUM_TABLES_MAX 50


static GPtrArray *tables = NULL;

/*
 * Returns a new vtable type, derived from @parent_type.
 *
 */
st_vtable_t *
st_vtable_register (st_vtable_t * parent_table, st_vtable_init_func_t init_func)
{
    st_vtable_t *table;

    g_assert (init_func != NULL);

    if (parent_table) {
	table = g_slice_new0 (st_vtable_t);
	*table = *parent_table;
	table->parent_table = parent_table;
    } else {
	table = g_slice_new0 (st_vtable_t);
	table->parent_table = NULL;
    }

    if (tables == NULL)
	tables = g_ptr_array_sized_new (30);
    g_ptr_array_add (tables, table);

    init_func (table);

    g_assert (table != parent_table);

    g_debug ("%p\n", table);

    return table;
}

bool
st_is_vtable (gpointer table)
{
    for (int i = 0; i < tables->len; i++) {

	if (table == g_ptr_array_index (tables, i))
	    return true;
    }


    return false;
}

static void
table_destroy (st_vtable_t * table, gpointer unused)
{
    g_slice_free (st_vtable_t, table);
}

void
st_mini_deregister_tables (void)
{
    g_ptr_array_foreach (tables, (GFunc) table_destroy, NULL);
    g_ptr_array_free (tables, TRUE);
    tables = NULL;
}
