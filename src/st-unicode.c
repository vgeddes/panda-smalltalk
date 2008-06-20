
#include "st-unicode.h"
#include "st-utils.h"
#include <string.h>

st_unichar
st_utf8_get_unichar (const char *p)
{
    st_unichar ch;

    if (p == NULL)
	return 0x00;
    
    if ((p[0] & 0x80) == 0x00) {
	ch = p[0];
    } else if ((p[0] & 0xe0) == 0xc0) {
	ch = ((p[0] & 0x1f) << 6) | (p[1] & 0x3f);
    } else if ((p[0] & 0xf0) == 0xe0) {
	ch = ((p[0] & 0xf) << 12) | ((p[1] & 0x3f) << 6) | (p[2] & 0x3f);
    } else if ((p[0] & 0xf8) == 0xf0) {
	ch = ((p[0] & 0x7) << 18) | ((p[1] & 0x3f) << 12) | ((p[2] & 0x3f) << 6) | (p[3] & 0x3f);
    } else
	ch = 0x00; /* undefined */

    return ch;
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
st_unichar_to_utf8 (st_unichar ch, char *outbuf)
{
    int bits;
    char *d = outbuf;
    
    if      (ch <       0x80) {  *d++ =  ch;                         bits = -6; }
    else if (ch <      0x800) {  *d++ = ((ch >>  6) & 0x1f) | 0xc0;  bits =  0; }
    else if (ch <    0x10000) {  *d++ = ((ch >> 12) & 0x0f) | 0xe0;  bits =  6; }
    else if (ch <   0x200000) {  *d++ = ((ch >> 18) & 0x07) | 0xf0;  bits = 12; }
    else if (ch <  0x4000000) {  *d++ = ((ch >> 24) & 0x03) | 0xf8;  bits = 18; }
    else if (ch < 0x80000000) {  *d++ = ((ch >> 30) & 0x01) | 0xfC;  bits = 24; }
    else return 0;

    for (; bits >= 0; bits -= 6) {
	*d++= ((ch >> bits) & 0x3F) | 0x80;
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
		return false;
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

const char *
st_utf8_offset_to_pointer (const char *string, st_uint offset)
{
    const char *p = string;

    for (st_uint i = 0; i < offset; i++)
	p = st_utf8_next_char (p);

    return p;
}

st_unichar *
st_utf8_to_ucs4 (const char *string)
{
    const st_uchar *p = string;
    st_unichar *buffer, c;
    st_uint     index = 0;

    if (string == NULL)
	return NULL;

    buffer = st_malloc (sizeof (st_unichar) * (st_utf8_strlen (string) + 1));

    while (p[0]) {	
	if ((p[0] & 0x80) == 0x00) {
	    c = p[0];
	    p += 1;
	} else if ((p[0] & 0xe0) == 0xc0) {
	    c = ((p[0] & 0x1f) << 6) | (p[1] & 0x3f);
	    p += 2;
	} else if ((p[0] & 0xf0) == 0xe0) {
	    c = ((p[0] & 0xf) << 12) | ((p[1] & 0x3f) << 6)  | (p[2] & 0x3f);
	    p += 3;
	} else if ((p[0] & 0xf8) == 0xf0) {
	    c = ((p[0] & 0x7) << 18) | ((p[1] & 0x3f) << 12) | ((p[2] & 0x3f) << 6) | (p[3] & 0x3f);
	    p += 4;
	} else
	    break;

	buffer[index++] = c;
    }

    buffer[index] = 0;
    return buffer;
}

#if 0

char *
st_ucs4_to_utf8 (const st_unichar *string)
{

    int bits;
    char *d = outbuf;
    
    if      (ch <       0x80) {  *d++ =  ch;                         bits = -6; }
    else if (ch <      0x800) {  *d++ = ((ch >>  6) & 0x1f) | 0xc0;  bits =  0; }
    else if (ch <    0x10000) {  *d++ = ((ch >> 12) & 0x0f) | 0xe0;  bits =  6; }
    else if (ch <   0x200000) {  *d++ = ((ch >> 18) & 0x07) | 0xf0;  bits = 12; }
    else if (ch <  0x4000000) {  *d++ = ((ch >> 24) & 0x03) | 0xf8;  bits = 18; }
    else if (ch < 0x80000000) {  *d++ = ((ch >> 30) & 0x01) | 0xfC;  bits = 24; }
    else return 0;

    for (; bits >= 0; bits -= 6) {
	*d++= ((ch >> bits) & 0x3F) | 0x80;
    }
    return d - outbuf;
}

#endif
