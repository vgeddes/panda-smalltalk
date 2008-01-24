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
    st_header_t header;

    st_oop_t size;

    guchar bytes[];

} st_byte_array_t;


st_oop_t  st_byte_array_size (st_oop_t object);

guchar   *st_byte_array_bytes (st_oop_t object);

guchar    st_byte_array_at (st_oop_t object, st_smi_t i);

void      st_byte_array_at_put (st_oop_t object, st_smi_t i, guchar value);

const st_vtable_t *st_byte_array_vtable (void);

#endif /* __ST_BYTE_ARRAY__ */
