/*
 * st-vtable.h
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

#ifndef __ST_VTABLE_H__
#define __ST_VTABLE_H__

#include <st-types.h>
#include <glib.h>

typedef struct
{
    /* allocation */
    st_oop (*allocate) (st_oop klass);

    st_oop (*allocate_arrayed) (st_oop klass, st_smi size);

    /* debugging */
    bool (*verify) (st_oop object);

    char *(*describe) (st_oop object);


    /* type tests */
    bool (*is_association) (void);

    bool (*is_class) (void);

    bool (*is_metaclass) (void);

    bool (*is_symbol) (void);

    bool (*is_float) (void);

    bool (*is_dictionary) (void);

    bool (*is_set) (void);

    bool (*is_compiled_method) (void);

    bool (*is_compiled_block) (void);

    bool (*is_block_closure) (void);

    bool (*is_method_context) (void);

    bool (*is_block_context) (void);

    bool (*is_arrayed) (void);

    bool (*is_array) (void);

    bool (*is_byte_array) (void);

    /* bootstrapping support below: Remove once an initial image has been created
     * We only use these for Strings, Symbols, Associations and Dictionaries
     */
    bool (*equal) (st_oop object, st_oop another);

    guint (*hash) (st_oop object);

    /* implementation methods for sets and dictionaries - also remove after bootstrapping */

    st_smi (*scan_for_object) (st_oop collection, st_oop object);

    void (*no_check_add) (st_oop collection, st_oop object);

} STVTable;

typedef void (*STVTableInitFunc) (STVTable *table);

const STVTable *st_vtable_register (const STVTable    *parent_table,
				    STVTableInitFunc   init_func);


#define ST_DEFINE_VTABLE(table_name, parent_table)                 \
                                                                   \
static void table_name##_vtable_init (STVTable *table);            \
                                                                   \
const STVTable *                                                   \
table_name##_vtable (void)                                         \
{                                                                  \
        static const STVTable *__st_vtable = NULL;                 \
        if (G_UNLIKELY (__st_vtable == NULL)) {                    \
                __st_vtable =                                      \
                st_vtable_register ((parent_table),                \
                                    table_name##_vtable_init);     \
        }                                                          \
	return __st_vtable;                                        \
}

#endif /* __ST_VTABLE_H__ */
