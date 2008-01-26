/*
 * st-vtable.c
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

#include "st-vtable.h"

#include <glib.h>

/* Max number of tables
 * please do increase if more tables are needed
 */ 
#define NUM_TABLES_MAX 15

static STVTable tables[NUM_TABLES_MAX] = { { 0 } };
static guint     num_tables = 0;

/*
 * Returns a new vtable, derived from a parent_table.
 */
const STVTable *
st_vtable_register (const STVTable     *parent_table,
		    STVTableInitFunc   init_func)
{
    STVTable * table;

    g_assert (init_func != NULL);
    g_assert (num_tables < NUM_TABLES_MAX);

    table = &tables[num_tables++];
    
    if (parent_table != NULL)
	*table = *parent_table;

    init_func (table);

    return table;
}

