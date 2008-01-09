/* 
 * st-mini.c
 *
 * Copyright (C) 2008 Vincent Geddes
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
st_vtable_register (st_vtable_t           *parent_table,
		    st_vtable_init_func_t  init_func)
{	
	st_vtable_t *table;

	g_assert (init_func != NULL);
	
	if (parent_table) {
		table = g_slice_new0  (st_vtable_t);
		*table = *parent_table;
		table->parent_table = parent_table;
	} else {	
		table = g_slice_new0  (st_vtable_t);
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
table_destroy (st_vtable_t *table, gpointer unused)
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

