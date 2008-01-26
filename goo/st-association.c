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

#include "st-association.h"
#include "st-object.h"

ST_DEFINE_VTABLE (st_association, st_heap_object_vtable ());


static guint
association_hash (st_oop object)
{
STAssociation *a = ST_ASSOCIATION (object);

    return st_object_hash (a->key) ^ st_object_hash (a->value);
}

static bool
association_equal (st_oop object, st_oop another)
{
    if (!st_object_is_association (another))
	return false;

    STAssociation *a = ST_ASSOCIATION (object);
    STAssociation *b = ST_ASSOCIATION (another);

    return st_object_equal (a->key, b->key) && st_object_equal (a->value, b->value);
}

static bool
is_association (void)
{
    return true;
}

static void
st_association_vtable_init (STVTable * table)
{
    /* ensure that STAssociation is not padded */
    assert_static (sizeof (STAssociation) == (sizeof (STHeader) + 2 * sizeof (st_oop)));

    table->is_association = is_association;

    table->equal = association_equal;
    table->hash = association_hash;
}

st_oop
st_association_new (st_oop key, st_oop value)
{
    st_oop assoc = st_object_new (st_association_class);

    st_association_set_key (assoc, key);
    st_association_set_value (assoc, value);

    return assoc;
}
