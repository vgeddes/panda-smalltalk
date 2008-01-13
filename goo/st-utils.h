/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/*
 * st-utils.h
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

#ifndef __ST_UTILS_H__
#define __ST_UTILS_H__

#include <glib.h>
#include <st-types.h>
#include <stdio.h>

/* bit utilities */
#define ST_NTH_BIT(n)       (1 << (n))
#define ST_NTH_MASK(n)      (ST_NTH_BIT(n) - 1)

/* an assertion that is checked at compile time */
#define assert_static(e)                \
   do {                                 \
      enum { assert_static__ = 1/(e) }; \
   } while (0)
   
#define streq(a,b)  (strcmp ((a),(b)) == 0)

/* returns the size of a type, in oop's */
#define ST_TYPE_SIZE(type) (sizeof (type) / sizeof (st_oop_t))

enum
{
    st_tag_mask = ST_NTH_MASK (2),
};

extern GList *objects;

st_oop_t st_allocate_object (gsize size);

#endif /* __ST_UTILS_H__ */
