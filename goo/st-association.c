/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/* 
 * st-association.c
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

#include <st-association.h>
#include <st-object.h>

ST_DEFINE_VTABLE (st_association, st_heap_object_vtable ())


     static guint association_hash (st_oop_t object)
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

    return st_object_equal (a->key, b->key) && st_object_equal (a->value, b->value);
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
st_association_vtable_init (st_vtable_t * table)
{
    /* ensure that st_association_t is not padded */
    assert_static (sizeof (st_association_t) == (sizeof (st_header_t) + 2 * sizeof (st_oop_t)));

    table->is_association = is_association;

    table->equal = association_equal;
    table->hash = association_hash;
    table->name = association_name;
}

st_oop_t
st_association_new (st_oop_t key, st_oop_t value)
{
    st_oop_t assoc = st_object_new (st_association_class);

    st_association_set_key (assoc, key);
    st_association_set_value (assoc, value);

    return assoc;
}
