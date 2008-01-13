/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/*
 * st-universe.c
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
#include "st-universe.h"
#include "st-utils.h"
#include "st-hashed-collection.h"
#include "st-symbol.h"

#include <string.h>

st_oop_t st_nil		= ST_OOP (NULL),
    st_true		= ST_OOP (NULL),
    st_false		= ST_OOP (NULL),
    st_symbol_table	= ST_OOP (NULL),
    st_smalltalk	= ST_OOP (NULL),
    st_undefined_object_class = ST_OOP (NULL),
    st_metaclass_class  = ST_OOP (NULL),
    st_behavior_class   = ST_OOP (NULL),
    st_smi_class	= ST_OOP (NULL),
    st_float_class      = ST_OOP (NULL),
    st_character_class  = ST_OOP (NULL),
    st_true_class       = ST_OOP (NULL),
    st_false_class      = ST_OOP (NULL),
    st_array_class      = ST_OOP (NULL),
    st_byte_array_class = ST_OOP (NULL),
    st_set_class	= ST_OOP (NULL),
    st_tuple_class      = ST_OOP (NULL),
    st_dictionary_class  = ST_OOP (NULL),
    st_association_class = ST_OOP (NULL),
    st_string_class = ST_OOP (NULL),
    st_symbol_class = ST_OOP (NULL),
    st_compiled_method_class = ST_OOP (NULL),
    st_compiled_block_class = ST_OOP (NULL);


st_oop_t
st_global_get (const char *name)
{
    return st_dictionary_at (st_smalltalk, st_symbol_new (name));
}

