/*
 * st-utils.h
 *
 * Copyright (C) 2008 Vincent Geddes
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
*/

#ifndef __ST_UTILS_H__
#define __ST_UTILS_H__

#include <glib.h>
#include <st-types.h>
#include <stdio.h>
#include <string.h>

/* bit utilities */
#define ST_NTH_BIT(n)       (1 << (n))
#define ST_NTH_MASK(n)      (ST_NTH_BIT(n) - 1)

/* A comile-time assertion */
#define assert_static(e)                \
   do {                                 \
      enum { assert_static__ = 1/(e) }; \
   } while (0)
   
#define streq(a,b)  (strcmp ((a),(b)) == 0)

/* returns the size of a type, in oop's */
#define ST_TYPE_SIZE(type) (sizeof (type) / sizeof (st_oop))

enum
{
    st_tag_mask = ST_NTH_MASK (2),
};

st_oop st_allocate_object (gsize size);

INLINE void
st_oops_copy (st_oop *to, st_oop *from, guint count)
{
    memmove (to, from, sizeof (st_oop) * count);
}


/* generic error object */
typedef struct {
    guint   code;
    char   *message;
    GData  *datalist;
} STError;

void     st_error_set        (STError   **error,
                              guint       code,
                              const char *message);

void     st_error_destroy    (STError *error);

gpointer st_error_get_data   (STError    *error,
			      const char *key);

void     st_error_set_data   (STError    *error,
			      const char *key,
			      gpointer    data);

#define ST_ERROR_LINE(error) (GPOINTER_TO_INT (st_error_get_data (error, "line")))

#endif /* __ST_UTILS_H__ */
