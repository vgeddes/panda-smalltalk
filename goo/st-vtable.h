/*
 * st-vtable.h
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

extern STVTable tables[];

typedef void (*STVTableInitFunc) (STVTable *table);

guint  st_vtable_register (guint            parent_table_id,
			   STVTableInitFunc init_func);


#define ST_DEFINE_VTABLE(table_name, parent_table_id)		   \
                                                                   \
static void table_name##_vtable_init (STVTable *table);	           \
 								   \
guint								   \
table_name##_vtable (void)				           \
{								   \
    static guint __st_vtable = 0;				   \
    if (__st_vtable == 0) {				           \
	__st_vtable =						   \
	    st_vtable_register (parent_table_id,	           \
				table_name##_vtable_init);	   \
    }							           \
    return __st_vtable;					           \
}

#endif /* __ST_VTABLE_H__ */
