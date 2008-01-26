/* 
 * st-byte-array.h
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

#ifndef __ST_BYTE_ARRAY_H__
#define __ST_BYTE_ARRAY_H__

#include <st-types.h>
#include <st-small-integer.h>
#include <st-heap-object.h>
#include <glib.h>

typedef struct
{
    STHeader header;

    st_oop size;

    guchar bytes[];

} STByteArray;


st_oop  st_byte_array_size (st_oop object);

guchar *st_byte_array_bytes (st_oop object);

guchar  st_byte_array_at (st_oop object, st_smi i);

void    st_byte_array_at_put (st_oop object, st_smi i, guchar value);

bool    st_byte_array_range_check (st_oop object, st_smi i);


const STVTable *st_byte_array_vtable (void);

#endif /* __ST_BYTE_ARRAY__ */
