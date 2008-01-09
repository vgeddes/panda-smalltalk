/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/* 
 * st-universe.c
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

#include <st-universe.h>
#include <st-utils.h>
#include <st-hashed-collection.h>
#include <st-symbol.h>

#include <string.h>

st_oop_t    st_nil                     = ST_OOP (NULL),
	    st_true                    = ST_OOP (NULL),
	    st_false                   = ST_OOP (NULL),
	    st_symbol_table            = ST_OOP (NULL),
	    st_smalltalk               = ST_OOP (NULL),
	    st_undefined_object_class  = ST_OOP (NULL),
	    st_metaclass_class         = ST_OOP (NULL),
	    st_behavior_class          = ST_OOP (NULL),
	    st_smi_class               = ST_OOP (NULL),
	    st_float_class             = ST_OOP (NULL),
	    st_character_class         = ST_OOP (NULL),
	    st_true_class              = ST_OOP (NULL),
	    st_false_class             = ST_OOP (NULL),
	    st_array_class             = ST_OOP (NULL),
	    st_byte_array_class        = ST_OOP (NULL),
	    st_set_class               = ST_OOP (NULL),
	    st_dictionary_class        = ST_OOP (NULL),
	    st_association_class       = ST_OOP (NULL),
	    st_string_class            = ST_OOP (NULL),
	    st_symbol_class            = ST_OOP (NULL),
	    st_compiled_method_class   = ST_OOP (NULL),
	    st_compiled_block_class    = ST_OOP (NULL);


st_oop_t
st_global_get (const char *name)
{
    return st_dictionary_at (st_smalltalk, st_symbol_new (name));
}

