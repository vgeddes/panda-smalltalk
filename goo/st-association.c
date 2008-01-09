/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/* 
 * st-association.c
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

#include <st-association.h>
#include <st-object.h>

ST_DEFINE_VTABLE (st_association, st_heap_object_vtable ())


static guint
association_hash (st_oop_t object)
{		
    st_association_t *a = _ST_ASSOCIATION (object);

    return st_object_hash (a->key) ^ st_object_hash (a->value);
}

static bool
association_equal (st_oop_t object, st_oop_t another)
{	
    if (!st_object_is_association (another))
	return false;
		
    st_association_t *a = _ST_ASSOCIATION (object);
    st_association_t *b = _ST_ASSOCIATION (another);

    return st_object_equal (a->key, b->key) &&
	   st_object_equal (a->value, b->value);
}

static bool
is_association (void)
{
    return true;
}

static const char *
association_name (void)
{
    static const char name[] = "Association";
    return name;
}

static void
st_association_vtable_init (st_vtable_t *table)
{
    /* ensure that st_association_t is not padded */
    assert_static (sizeof (st_association_t) == (sizeof (st_header_t) + 2 * sizeof (st_oop_t)));

    table->is_association      = is_association;	

    table->equal               = association_equal;
    table->hash                = association_hash;
    table->name                = association_name;
}

st_oop_t
st_association_new (st_oop_t key, st_oop_t value)
{
    st_oop_t assoc = st_object_new (st_association_class);
    
    st_association_set_key (assoc, key);
    st_association_set_value (assoc, value);
    
    return assoc;
}


