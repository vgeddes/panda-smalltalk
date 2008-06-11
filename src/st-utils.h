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

#include <st-types.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/* bit utilities */
#define ST_NTH_BIT(n)         (1 << (n))
#define ST_NTH_MASK(n)        (ST_NTH_BIT(n) - 1)

#define ST_N_ELEMENTS(static_array)  (sizeof(static_array) / sizeof ((static_array) [0]))

#define ST_DIR_SEPARATOR   '/'
#define ST_DIR_SEPARATOR_S "/"

#ifdef __GNUC__
#define ST_GNUC_MALLOC
#define ST_GNUC_PRINTF(format_index, argument_index)
#else
#define ST_GNUC_MALLOC __attribute__ ((malloc))
#define ST_GNUC_PRINTF(format_index, argument_index)  __attribute__ ((format (printf, format_index, argument_index)))
#endif

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

st_oop st_allocate_object (st_uint size);

INLINE void
st_oops_copy (st_oop *to, st_oop *from, st_uint count)
{
    memmove (to, from, sizeof (st_oop) * count);
}

st_pointer  st_malloc  (size_t size) ST_GNUC_MALLOC;
st_pointer  st_malloc0 (size_t size) ST_GNUC_MALLOC;
void        st_free    (st_pointer mem);

#define st_new(struct_type)  ((struct_type *) st_malloc  (sizeof (struct_type)))
#define st_new0(struct_type) ((struct_type *) st_malloc0 (sizeof (struct_type)))

bool    st_file_get_contents (const char *filename,
			      char      **buffer);

char  *st_strdup         (const char *string);
char  *st_strdup_printf  (const char *format, ...) ST_GNUC_PRINTF (1, 2);
char  *st_strdup_vprintf (const char *format, va_list args);
char  *st_strconcat      (const char *first, ...);

typedef st_uint st_unichar;

#define st_utf8_skip(c) (((0xE5000000 >> (((c) >> 3) & 0xFE)) & 3) + 1)
#define st_utf8_next_char(p) (char *)((p) + st_utf8_skip (*(const char *)(p)))

int         st_utf8_strlen            (const char *string);
st_unichar  st_utf8_get_unichar       (const char *p);
bool        st_utf8_validate          (const char *string, ssize_t max_len);
int         st_unichar_to_utf8        (st_unichar ch, char *outbuf);
const char *st_utf8_offset_to_pointer (const char *string, st_uint offset);
st_unichar *st_utf8_to_ucs4           (const char *string);

typedef struct st_list st_list;

struct st_list
{
    st_pointer data;
    st_list   *next;
};

typedef void (* st_list_foreach_func) (st_pointer data); 

st_list  *st_list_append  (st_list *list,  st_pointer data);
st_list  *st_list_prepend (st_list *list,  st_pointer data);
st_list  *st_list_concat  (st_list *list1, st_list *list2);
void      st_list_foreach (st_list *list,  st_list_foreach_func func); 
st_list  *st_list_reverse (st_list *list);
st_uint   st_list_length  (st_list *list);
void      st_list_destroy (st_list *list);

#if  defined(__GNUC__) && defined(__OPTIMIZE__)
#define ST_LIKELY(condition)     __builtin_expect (!!(condition), 1)
#define ST_UNLIKELY(condition)   __builtin_expect (!!(condition), 0)
#else
#define ST_LIKELY(condition)     condition
#define ST_UNLIKELY(condition)   condition
#endif

#ifdef __GNUC__
#define ST_STMT_START  ({
#define ST_STMT_END    })
#else
#define ST_STMT_START  do {
#define ST_STMT_END    } while (0)
#endif

#ifndef ST_DEBUG
#define st_assert(condition) 
#else
#define st_assert(condition)						\
ST_STMT_START						         	\
if (!(condition)) {						       	\
    fprintf (stderr, "%s:%i: %s: assertion `" #condition "' failed\n",	\
	     __FILE__, __LINE__, __FUNCTION__);				\
    abort ();								\
 }									\
ST_STMT_END
#endif

#ifndef ST_DEBUG
#define st_assert_not_reached() 
#else
#define st_assert_not_reached()						\
ST_STMT_START						        	\
fprintf (stderr, "%s:%i: %s: should not reach here\n",			\
	 __FILE__, __LINE__, __FUNCTION__);				\
abort ();								\
ST_STMT_END
#endif

#endif /* __ST_UTILS_H__ */
