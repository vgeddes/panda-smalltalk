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

G_BEGIN_DECLS
#define ST_ADD_BITS(n, m)   ((n) | (m))
#define ST_SET_BITS(n, m)   ((n) |= (m))
#define ST_MASK_BITS(n, m)  ((n) & (m))
#define ST_NTH_BIT(n)       (1 << (n))
#define ST_NTH_MASK(n)      (ST_NTH_BIT(n) - 1)
#define assert_static(e)                \
   do {                                 \
      enum { assert_static__ = 1/(e) }; \
   } while (0)
#define streq(a,b)  (strcmp ((a),(b)) == 0)
    enum
{
    st_tag_mask = ST_NTH_MASK (2),
};

extern GList *objects;

st_oop_t st_allocate_object (gsize size);


G_END_DECLS
#endif /* __ST_UTILS_H__ */
