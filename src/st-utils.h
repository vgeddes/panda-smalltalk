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

#if 1
#define ST_ROUNDING_MASK 0x7
#else
#define ST_ROUNDING_MASK 0x3
#endif

#define ST_BYTES_TO_OOPS(m) ((m) / sizeof (st_oop))
#define ST_OOPS_TO_BYTES(m) ((m) * sizeof (st_oop))

#define ST_ROUND_BYTES(size) (((size) ^ ((size) & ST_ROUNDING_MASK)) + (!!((size) & ST_ROUNDING_MASK) * sizeof (st_pointer)))

#define ST_ROUNDED_UP_OOPS(m) (ST_BYTES_TO_OOPS ((m) + ST_OOPS_TO_BYTES (1) - 1))

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

#ifndef MAX
#define MAX(a,b) (((a)>(b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b) (((a)<(b)) ? (a) : (b))
#endif

#ifndef CLAMP
#define CLAMP(a,low,high) (((a) < (low)) ? (low) : (((a) > (high)) ? (high) : (a)))
#endif

enum
{
    st_tag_mask = ST_NTH_MASK (2),
};

INLINE void
st_oops_copy (st_oop *to, st_oop *from, st_uint count)
{
    memcpy (to, from, sizeof (st_oop) * count);
}

INLINE void
st_oops_move (st_oop *to, st_oop *from, st_uint count)
{
    memmove (to, from, sizeof (st_oop) * count);
}

st_pointer  st_malloc   (size_t size) ST_GNUC_MALLOC;
st_pointer  st_malloc0  (size_t size) ST_GNUC_MALLOC;
st_pointer  st_realloc  (st_pointer mem, size_t size);

void        st_free    (st_pointer mem);

#define st_new(struct_type)  ((struct_type *) st_malloc  (sizeof (struct_type)))
#define st_new0(struct_type) ((struct_type *) st_malloc0 (sizeof (struct_type)))

bool    st_file_get_contents (const char *filename,
			      char      **buffer);

char  *st_strdup         (const char *string);
char  *st_strdup_printf  (const char *format, ...) ST_GNUC_PRINTF (1, 2);
char  *st_strdup_vprintf (const char *format, va_list args);
char  *st_strconcat      (const char *first, ...);

char ** st_strsplit (const char *string, const char *delimiter, int max_tokens);
void    st_strfreev (char **str_array);

char   *st_strndup (const char *str, size_t n);

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

typedef struct st_bit_array st_bit_array;

struct st_bit_array {
    
    st_uchar *data;
    st_ulong  data_size;
};

st_bit_array *st_bit_array_new     (st_ulong size, bool clear);

st_ulong      st_bit_array_size    (st_bit_array *array);

void          st_bit_array_clear   (st_bit_array *array);

void          st_bit_array_destroy (st_bit_array *array);

INLINE bool
st_bit_array_get (st_bit_array *array, st_ulong index)
{
    return (array->data[index >> 3] >> (index & 0x7)) & 1;
}

INLINE void
st_bit_array_set (st_bit_array *array, st_ulong index)
{   
    array->data[index >> 3] |= 1 << (index & 0x7);
}

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
