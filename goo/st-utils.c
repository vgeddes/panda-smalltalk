/*
 * st-utils.c
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
 
#include "st-utils.h"
#include "st-array.h"
#include "st-byte-array.h"
#include "st-virtual-space.h"

#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <error.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>

int vasprintf(char **strp, const char *fmt, va_list ap);

extern STVirtualSpace *st_virtual_space;
extern char *program_invocation_short_name;


st_oop
st_allocate_object (gsize size)
{
    static st_oop *mark = NULL;
    st_oop *object;

    st_assert (size >= 2);
    
    if (ST_UNLIKELY (mark == NULL)) {
	mark = st_virtual_space->start;
    }

    if (ST_UNLIKELY ((mark + size) >= st_virtual_space->end))
	abort ();

    object = mark;    
    mark = mark + size;

    return ST_OOP (object);
}

st_pointer
st_malloc (size_t size)
{
    void *ptr;

    ptr = malloc (size);
    if (ST_UNLIKELY (ptr == NULL))
	abort ();

    return ptr;
}

st_pointer
st_malloc0 (size_t size)
{
    void *ptr;

    ptr = calloc (1, size);
    if (ST_UNLIKELY (ptr == NULL))
	abort ();

    return ptr;
}

void
st_free (st_pointer mem)
{
    if (ST_UNLIKELY (mem != NULL))
	free (mem);
}

bool
st_file_get_contents (const char *filename,
		      char      **buffer)
{
    struct stat info;
    size_t  total;
    ssize_t count;
    char   *temp;
    int     fd;

    st_assert (filename != NULL);

    *buffer = NULL;

    if (stat (filename, &info) != 0) {
	fprintf (stderr, "%s: error: `%s': %s\n", program_invocation_short_name, filename, strerror (errno));
	return false;
    }

    if (!S_ISREG (info.st_mode)) {
	fprintf (stderr, "%s: error: `%s': Not a regular file\n", program_invocation_short_name, filename);
	return false;	
    }

    fd = open (filename, O_RDONLY);
    if (fd < 0) {
	fprintf (stderr, "%s: error: `%s': %s\n", program_invocation_short_name, filename, strerror (errno));
	return false;
    }

    total = 0;
    temp = st_malloc (info.st_size + 1);
    while (total < info.st_size) {
	count = read (fd, temp + total, info.st_size - total);
	if (count < 0) {
	    fprintf (stderr, "%s: error: %s: %s\n", program_invocation_short_name, filename, strerror (errno));
	    st_free (temp);
	    return false;
	}
	total += count;
    }

    temp[info.st_size] = 0;
    *buffer = temp;

    return true;
}

/* Derived from eglib (part of Mono)
 * Copyright (C) 2006 Novell, Inc.
 */
char *
st_strdup (const char *string)
{
    size_t  size;
    char   *copy;

    st_assert (string != NULL);

    size = strlen (string);
    copy = st_malloc (size + 1);
    strcpy (copy, string);

    return copy;
}

/* Derived from eglib (part of Mono)
 * Copyright (C) 2006 Novell, Inc.
 */
char *
st_strdup_vprintf (const gchar *format, va_list args)
{
	int n;
	char *ret;
	
	n = vasprintf (&ret, format, args);
	if (n == -1)
		return NULL;

	return ret;
}

/* Derived from eglib (part of Mono)
 * Copyright (C) 2006 Novell, Inc.
 */
char *
st_strdup_printf (const gchar *format, ...)
{
	char *ret;
	va_list args;
	int n;

	va_start (args, format);
	n = vasprintf (&ret, format, args);
	va_end (args);
	if (n == -1)
		return NULL;

	return ret;
}

/* Derived from eglib (part of Mono)
 * Copyright (C) 2006 Novell, Inc.
 */
char *
st_strconcat (const char *first, ...)
{
	va_list args;
	size_t total = 0;
	char *s, *ret;

	total += strlen (first);
	va_start (args, first);
	for (s = va_arg (args, char *); s != NULL; s = va_arg(args, char *)){
		total += strlen (s);
	}
	va_end (args);
	
	ret = g_malloc (total + 1);
	if (ret == NULL)
		return NULL;

	ret [total] = 0;
	strcpy (ret, first);
	va_start (args, first);
	for (s = va_arg (args, char *); s != NULL; s = va_arg(args, char *)){
		strcat (ret, s);
	}
	va_end (args);

	return ret;
}



st_list *
st_list_append  (st_list *list, st_pointer data)
{
    st_list *new_list, *l;
    
    new_list = st_new (st_list);
    new_list->data = data;
    new_list->next = NULL;

    if (list == NULL)
	return new_list;

    l = list;
    while (l->next)
	l = l->next;

    l->next = new_list;

    return list;
}

st_list *
st_list_prepend (st_list *list, st_pointer data)
{
    st_list *new_list;
    
    new_list = st_new (st_list);
    new_list->data = data;
    new_list->next = list;

    return new_list;
}

st_list *
st_list_reverse (st_list *list)
{
    st_list *next, *prev = NULL;
    
    while (list) {
	next = list->next;

	list->next = prev;

	prev = list;
	list = next;
    }

    return prev;
}

st_list *
st_list_concat  (st_list *list1, st_list *list2)
{
    st_list *l;

    if (list1 == NULL)
	return list2;

    if (list2 == NULL)
	return list1;
    
    l = list1;
    while (l->next)
	l = l->next;
    l->next = list2;
     
    return list1;
} 

st_uint
st_list_length (st_list *list)
{
    st_list *l = list;
    st_uint len = 0;

    for (; l; l = l->next)
	++len;

    return len;
}

void
st_list_foreach (st_list *list, st_list_foreach_func func)
{
    for (st_list *l = list; l; l = l->next)
	func (l->data);
}

void
st_list_destroy (st_list *list)
{
    st_list *next, *current;

    current = list;

    while (current) {
	next = current->next;
	st_free (current);
	current = next;
    }
}

static int
utf8_char_length (const char *p)
{
    st_uint lead = *p;
    if (lead < 0x80) 
	return 1;
    else if ((lead >> 5) == 0x6)
	return 2;
    else if ((lead >> 4) == 0xe)
	return 3;
    else if ((lead >> 3) == 0x1e)
	return 4;
    else 
	return 0;
}

wchar_t
st_utf8_get_unichar (const char *p)
{
    wchar_t cp = *p;
    int length = utf8_char_length (p);

    switch (length) {
    case 1:
	break;
    case 2:
	p++;
	cp = ((cp << 6) & 0x7ff) + ((*p) & 0x3f);
	break;
    case 3:
	++p; 
	cp = ((cp << 12) & 0xffff) + ((*p << 6) & 0xfff);
	++p;
	cp += (*p) & 0x3f;
	break;
    case 4:
	++p;
	cp = ((cp << 18) & 0x1fffff) + ((*p << 12) & 0x3ffff);                
	++p;
	cp += ((*p) << 6) & 0xfff;
	++p;
	cp += (*p) & 0x3f; 
	break;
    }
    
    return cp;        
}

/*
 * Copyright (C) 2008 Colin Percival
 */
#if 0
#define ONEMASK ((size_t)(-1) / 0xFF)
size_t
st_utf8_strlen(const char * _s)
{
	const char * s;
	size_t count = 0;
	size_t u;
	unsigned char b;

	/* Handle any initial misaligned bytes. */
	for (s = _s; (uintptr_t)(s) & (sizeof(size_t) - 1); s++) {
		b = *s;

		/* Exit if we hit a zero byte. */
		if (b == '\0')
			goto done;

		/* Is this byte NOT the first byte of a character? */
		count += (b >> 7) & ((~b) >> 6);
	}

	/* Handle complete blocks. */
	for (; ; s += sizeof(size_t)) {
		/* Prefetch 256 bytes ahead. */
		__builtin_prefetch(&s[256], 0, 0);

		/* Grab 4 or 8 bytes of UTF-8 data. */
		u = *(size_t *)(s);

		/* Exit the loop if there are any zero bytes. */
		if ((u - ONEMASK) & (~u) & (ONEMASK * 0x80))
			break;

		/* Count bytes which are NOT the first byte of a character. */
		u = ((u & (ONEMASK * 0x80)) >> 7) & ((~u) >> 6);
		count += (u * ONEMASK) >> ((sizeof(size_t) - 1) * 8);
	}

	/* Take care of any left-over bytes. */
	for (; ; s++) {
		b = *s;

		/* Exit if we hit a zero byte. */
		if (b == '\0')
			break;

		/* Is this byte NOT the first byte of a character? */
		count += (b >> 7) & ((~b) >> 6);
	}

done:
	return ((s - _s) - count);
}
#endif

/* Derived from FontConfig
 * Copyright (C) 2006 Keith Packard
 */
int
st_unichar_to_utf8 (wchar_t unichar, char *outbuf)
{
    int bits;
    char *d = outbuf;
    
    if      (unichar <       0x80) {  *d++ =  unichar;                         bits = -6; }
    else if (unichar <      0x800) {  *d++ = ((unichar >>  6) & 0x1F) | 0xC0;  bits =  0; }
    else if (unichar <    0x10000) {  *d++ = ((unichar >> 12) & 0x0F) | 0xE0;  bits =  6; }
    else if (unichar <   0x200000) {  *d++ = ((unichar >> 18) & 0x07) | 0xF0;  bits = 12; }
    else if (unichar <  0x4000000) {  *d++ = ((unichar >> 24) & 0x03) | 0xF8;  bits = 18; }
    else if (unichar < 0x80000000) {  *d++ = ((unichar >> 30) & 0x01) | 0xFC;  bits = 24; }
    else return 0;

    for (; bits >= 0; bits -= 6) {
	*d++= ((unichar >> bits) & 0x3F) | 0x80;
    }
    return d - outbuf;
}

/**
 * st_utf8_validate
 * @utf: Pointer to putative UTF-8 encoded string.
 *
 * Checks @utf for being valid UTF-8. @utf is assumed to be
 * null-terminated. This function is not super-strict, as it will
 * allow longer UTF-8 sequences than necessary. Note that Java is
 * capable of producing these sequences if provoked. Also note, this
 * routine checks for the 4-byte maximum size, but does not check for
 * 0x10ffff maximum value.
 *
 * Return value: true if @utf is valid.
 **/
/* Derived from eglib, libxml2
 * Copyright (C) 2006 Novell, Inc.
 * Copyright (C) 1998-2003 Daniel Veillard
 */
bool
st_utf8_validate (const char *string, ssize_t max_len)
{
    int ix;

    if (max_len == -1)
	max_len = strlen (string);
	
    /*
     * input is a string of 1, 2, 3 or 4 bytes.  The valid strings
     * are as follows (in "bit format"):
     *    0xxxxxxx                                      valid 1-byte
     *    110xxxxx 10xxxxxx                             valid 2-byte
     *    1110xxxx 10xxxxxx 10xxxxxx                    valid 3-byte
     *    11110xxx 10xxxxxx 10xxxxxx 10xxxxxx           valid 4-byte
     */
    for (ix = 0; ix < max_len;) {      /* string is 0-terminated */
	st_uchar c;
		
	c = string[ix];
	if ((c & 0x80) == 0x00) {	/* 1-byte code, starts with 10 */
	    ix++;
	} else if ((c & 0xe0) == 0xc0) {/* 2-byte code, starts with 110 */
	    if (((ix+1) >= max_len) || (string[ix+1] & 0xc0 ) != 0x80)
		return false;
	    ix += 2;
	} else if ((c & 0xf0) == 0xe0) {/* 3-byte code, starts with 1110 */
	    if (((ix + 2) >= max_len) || 
		((string[ix+1] & 0xc0) != 0x80) ||
		((string[ix+2] & 0xc0) != 0x80))
		return FALSE;
	    ix += 3;
	} else if ((c & 0xf8) == 0xf0) {/* 4-byte code, starts with 11110 */
	    if (((ix + 3) >= max_len) ||
		((string[ix+1] & 0xc0) != 0x80) ||
		((string[ix+2] & 0xc0) != 0x80) ||
		((string[ix+3] & 0xc0) != 0x80))
		return false;
	    ix += 4;
	} else {/* unknown encoding */
	    return false;
	}
    }
	
    return true;
}

/**
 * st_utf8_strlen:
 * @utf:  a sequence of UTF-8 encoded bytes
 *
 * compute the length of an UTF8 string, it doesn't do a full UTF8
 * checking of the content of the string.
 *
 * Returns the number of characters in the string or -1 in case of error
 */
/* Derived from libxml2
 * Copyright (C) 1998-2003 Daniel Veillard
 */
int
st_utf8_strlen (const char *string)
{
    int ret = 0;

    if (string == NULL)
        return(-1);

    while (*string != 0) {
        if (string[0] & 0x80) {
            if ((string[1] & 0xc0) != 0x80)
                return(-1);
            if ((string[0] & 0xe0) == 0xe0) {
                if ((string[2] & 0xc0) != 0x80)
                    return(-1);
                if ((string[0] & 0xf0) == 0xf0) {
                    if ((string[0] & 0xf8) != 0xf0 || (string[3] & 0xc0) != 0x80)
                        return(-1);
                    string += 4;
                } else {
                    string += 3;
                }
            } else {
                string += 2;
            }
        } else {
            string++;
        }
        ret++;
    }
    return(ret);
}

/* Derived from libxml2
 * Copyright (C) 1998-2003 Daniel Veillard
 */
const char *
st_utf8_char_at_pos (const char *string, int pos)
{
    char ch;

    if (string == NULL) return(NULL);
    if (pos < 0)
        return(NULL);
    while (pos--) {
        if ((ch=*string++) == 0) return(NULL);
        if ( ch & 0x80 ) {
            /* if not simple ascii, verify proper format */
            if ( (ch & 0xc0) != 0xc0 )
                return(NULL);
            /* then skip over remaining bytes for this char */
            while ( (ch <<= 1) & 0x80 )
                if ( (*string++ & 0xc0) != 0x80 )
                    return(NULL);
        }
    }
    return ((char *) string);
}

