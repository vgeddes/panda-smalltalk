/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/*
 * st-symbol.c
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

#include <st-symbol.h>
#include <st-hashed-collection.h>
#include <st-byte-array.h>

#include <string.h>

ST_DEFINE_VTABLE (st_symbol, st_byte_array_vtable ())

     static bool symbol_equal (st_oop_t object, st_oop_t another)
{
    if (object == another)
	return true;

    if (st_object_class (object) == st_object_class (another))
	return false;

    // now just do a string comparison
    if (st_object_super_virtual (object)->equal (object, another))
	return true;

    return false;
}

static bool
is_symbol (void)
{
    return true;
}

static void
st_symbol_vtable_init (st_vtable_t * table)
{
    table->equal = symbol_equal;

    table->is_symbol = is_symbol;
}

static st_oop_t
string_new (st_oop_t klass, const char *bytes)
{
    int len = strlen (bytes);

    st_oop_t string = st_object_new_arrayed (klass, len);

    guchar *data = st_byte_array_bytes (string);

    memcpy (data, bytes, len);

    return string;
}

st_oop_t
st_string_new (const char *bytes)
{
    return string_new (st_string_class, bytes);

}

st_oop_t
st_symbol_new (const char *bytes)
{
    st_oop_t element = st_set_like (st_symbol_table, st_string_new (bytes));

    if (element == st_nil) {

	st_oop_t symbol = string_new (st_symbol_class, bytes);

	st_set_add (st_symbol_table, symbol);

	return symbol;
    }

    return element;
}
