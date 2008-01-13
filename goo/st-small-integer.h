/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/*
 * st-small-integer.h
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

#ifndef _ST_SMALL_INTEGER_H__
#define _ST_SMALL_INTEGER_H__

#include <st-types.h>
#include <st-vtable.h>

INLINE st_oop_t st_smi_new (st_smi_t num);
INLINE st_smi_t st_smi_value (st_oop_t smi);
INLINE st_oop_t st_smi_increment (st_oop_t smi);
INLINE st_oop_t st_smi_decrement (st_oop_t smi);
INLINE bool st_smi_equal (st_oop_t m, st_oop_t n);
INLINE st_smi_t st_smi_hash (st_oop_t smi);
const st_vtable_t *st_smi_vtable (void);


/* inline definitions */

INLINE st_oop_t
st_smi_new (st_smi_t num)
{
    return (st_oop_t) (num << ST_TAG_SIZE);
}

INLINE st_smi_t
st_smi_value (st_oop_t smi)
{
    return (st_smi_t) (smi >> ST_TAG_SIZE);
}

INLINE st_oop_t
st_smi_increment (st_oop_t smi)
{
    return st_smi_new (st_smi_value (smi) + 1);
}

INLINE st_oop_t
st_smi_decrement (st_oop_t smi)
{
    return st_smi_new (st_smi_value (smi) - 1);
}

INLINE bool
st_smi_equal (st_oop_t m, st_oop_t n)
{
    return m == n;
}

INLINE st_smi_t
st_smi_hash (st_oop_t smi)
{
    return smi;
}

#endif /* _ST_SMALL_INTEGER_H__ */
