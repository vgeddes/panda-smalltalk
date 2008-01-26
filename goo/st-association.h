/* 
 * st-association.h
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

#ifndef __ST_ASSOCIATION_H__
#define __ST_ASSOCIATION_H__

#include <st-heap-object.h>
#include <st-types.h>
#include <st-vtable.h>

typedef struct
{
    STHeader header;

    st_oop key;
    st_oop value;

} STAssociation;

st_oop st_association_new (st_oop key, st_oop value);

INLINE st_oop st_association_key (st_oop assoc);

INLINE st_oop st_association_value (st_oop assoc);

INLINE void st_association_set_key (st_oop assoc, st_oop key);

INLINE void st_association_set_value (st_oop assoc, st_oop value);

const STVTable *st_association_vtable (void);

/* inline definitions */
#define ST_ASSOCIATION(oop) ((STAssociation *) ST_POINTER (oop))

INLINE st_oop
st_association_key (st_oop assoc)
{
    return ST_ASSOCIATION (assoc)->key;
}

INLINE st_oop
st_association_value (st_oop assoc)
{
    return ST_ASSOCIATION (assoc)->value;
}

INLINE void
st_association_set_key (st_oop assoc, st_oop key)
{
    ST_ASSOCIATION (assoc)->key = key;
}

INLINE void
st_association_set_value (st_oop assoc, st_oop value)
{
    ST_ASSOCIATION (assoc)->value = value;
}

#endif /* __ST_ASSOCIATION_H__ */
