/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/*
 * st-mini.h
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

#ifndef __ST_MINI_H__
#define __ST_MINI_H__

#include <st-types.h>
#include <glib.h>

typedef struct _st_vtable st_vtable_t;

struct _st_vtable
{
    st_vtable_t *parent_table;

    /* allocation */
    st_oop_t (*allocate) (st_oop_t klass);

    st_oop_t (*allocate_arrayed) (st_oop_t klass, st_smi_t size);


    /* debugging */
      bool (*verify) (st_oop_t object);

    const char *(*name) (void);

    char *(*describe) (st_oop_t object);


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
     * We only use these for Strings, Symbols and Associations and Dictionaries
     */
      bool (*equal) (st_oop_t object, st_oop_t another);

      guint (*hash) (st_oop_t object);

    /* implementation methods for set's and dictionaries - also remove after bootstrapping */

    st_smi_t (*scan_for_object) (st_oop_t collection, st_oop_t object);

    void (*no_check_add) (st_oop_t collection, st_oop_t object);

};

typedef void (*st_vtable_init_func_t) (st_vtable_t * table);

st_vtable_t *st_vtable_register (st_vtable_t * parent_table, st_vtable_init_func_t init_func);

void st_deregister_vtables (void);

bool st_is_vtable (gpointer table);

#define printage(name) "name"

#define ST_DEFINE_VTABLE(table_name, parent_table)                 \
                                                                   \
static void table_name##_vtable_init (st_vtable_t *table);         \
                                                                   \
st_vtable_t *                                                      \
table_name##_vtable (void)                                         \
{                                                                  \
        static st_vtable_t *__st_vtable = NULL;                    \
        if (G_UNLIKELY (__st_vtable == NULL)) {                    \
                __st_vtable =                                      \
                st_vtable_register ((parent_table),                \
                                    table_name##_vtable_init);     \
        }                                                          \
	return __st_vtable;                                        \
}

#endif /* __ST_MINI_H__ */
