/*
 * st-vtable.c
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

#include "st-vtable.h"

#include <glib.h>

/* Max number of tables
 * please do increase if more tables are needed
 */ 
#define NUM_TABLES_MAX 50

STVTable tables[NUM_TABLES_MAX] = { { 0 } };
static guint num_tables = 1;

/*
 * Creates a new vtable, derived from a parent_table.
 */
guint
st_vtable_register (guint             parent_table_id,
		    STVTableInitFunc  init_func)
{
    int table_id;
    g_debug ("%i\n", num_tables);

    g_assert (init_func != NULL);
    g_assert (parent_table_id < NUM_TABLES_MAX);
    g_assert (num_tables < NUM_TABLES_MAX);

    table_id = num_tables++;
    
    if (parent_table_id > 0)
	tables[table_id] = tables[parent_table_id];

    init_func (&tables[table_id]);

    return table_id;
}

