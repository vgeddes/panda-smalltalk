/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */

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

enum {
	st_tag_mask = ST_NTH_MASK (2),
};

extern GList *objects;

st_oop_t st_allocate_object (gsize size);


G_END_DECLS

#endif /* __ST_UTILS_H__ */
