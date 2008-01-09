/* 
 * st-mini.h
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

#ifndef __ST_MINI_H__
#define __ST_MINI_H__

#include <st-types.h>
#include <glib.h>

typedef struct _st_vtable st_vtable_t;

struct _st_vtable
{
	st_vtable_t *parent_table;
	
	/* allocation */
	st_oop_t  (* allocate)          (st_oop_t klass);
	
	st_oop_t  (* allocate_arrayed)  (st_oop_t klass, st_smi_t size);
	
	
	/* debugging */	
	bool (* verify)                 (st_oop_t object);
	
	const char * (* name)       (void);	

	char * (* describe)           (st_oop_t object);
	
	
	/* type tests */
	bool (* is_association)     (void);
	
	bool (* is_class)           (void);
	
	bool (* is_metaclass)       (void);
	
	bool (* is_symbol)          (void);
	
	bool (* is_float)           (void);
	
	bool (* is_dictionary)      (void);
	
	bool (* is_set)             (void);
	
	bool (* is_compiled_method) (void);
	
	bool (* is_compiled_block)  (void);

	bool (* is_block_closure)   (void);
	
	bool (* is_method_context)  (void);
	
	bool (* is_block_context)   (void);
	
	bool (* is_arrayed)         (void);
	
	bool (* is_array)           (void);
	
	bool (* is_byte_array)      (void);
	

	/* bootstrapping support below: Remove once an initial image has been created
	 * We only use these for Strings, Symbols and Associations and Dictionaries
	 */
	bool (* equal)              (st_oop_t object, st_oop_t another);
	
	guint  (* hash)             (st_oop_t object);
	
	/* implementation methods for set's and dictionaries - also remove after bootstrapping */
	
	long (* scan_for_object)        (st_oop_t collection, st_oop_t object);
	
	void (* no_check_add)           (st_oop_t collection, st_oop_t object);
	
};

typedef void (*st_vtable_init_func_t) (st_vtable_t *table);      

st_vtable_t   *st_vtable_register    (st_vtable_t           *parent_table,
			              st_vtable_init_func_t  init_func);
			                       
void           st_deregister_vtables (void);

bool	       st_is_vtable	     (gpointer table);

#define printage(name) "name"

#define ST_DEFINE_VTABLE(table_name, parent_table)                 \
                                                                   \
static void table_name##_vtable_init (st_vtable_t *table);         \
                                                                   \
st_vtable_t *                                                      \
table_name##_vtable (void)                                         \
{   \
    	                                    \
        static st_vtable_t *__st_vtable = NULL;                    \
        if (G_UNLIKELY (__st_vtable == NULL)) {                    \
                      g_debug ( #table_name "\n" ); \
                __st_vtable =                                      \
                st_vtable_register ((parent_table),                \
                                    table_name##_vtable_init);     \
                                                                   \
                                                \
        }                                                          \
	return __st_vtable;                                        \
}

#endif /* __ST_MINI_H__ */
