/*
 * st-large-integer.c
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

#include "st-large-integer.h"
#include "st-universe.h"
#include "st-types.h"
#include "math.h"

#define VALUE(oop)             (&(ST_LARGE_INTEGER(oop)->value))

st_oop
st_large_integer_new_from_string (const char *string, st_uint radix)
{
    mp_int value;
    int result;

    st_assert (string != NULL);

    result = mp_init (&value);
    if (result != MP_OKAY)
	goto out;

    result = mp_read_radix (&value, string, radix);
    if (result != MP_OKAY)
	goto out;

    return st_large_integer_new (&value);

  out:
    mp_clear (&value);
    fprintf (stderr, mp_error_to_string (result));
    return st_nil;
}

char *
st_large_integer_to_string (st_oop integer, st_uint radix)
{
    int result;
    int size;

    result = mp_radix_size (VALUE (integer), radix, &size);
    if (result != MP_OKAY)
	goto out;

    char *str = st_malloc (size);

    mp_toradix (VALUE (integer), str, radix);
    if (result != MP_OKAY)
	goto out;

    return str;

  out:
    fprintf (stderr, mp_error_to_string (result));
    return NULL;
}

static st_oop
allocate_with_value (st_space *space, st_oop class, mp_int * value)
{
    st_oop object;

    object = st_space_allocate_object (space, class, sizeof (struct st_large_integer) / sizeof (st_oop));

    if (value)
	*VALUE (object) = *value;
    else
	mp_init (VALUE (object));

    return object;
}

st_oop
st_large_integer_new (mp_int * value)
{
    return allocate_with_value (om->moving_space, st_large_integer_class, value);
    
}

static st_oop
allocate (st_space *space, st_oop class)
{
    return allocate_with_value (space, class, NULL);
}


static st_oop
large_integer_copy (st_oop object)
{
    mp_int value;
    int result;

    result = mp_init_copy (&value, VALUE (object));
    if (result != MP_OKAY)
	st_assert_not_reached ();

    return st_large_integer_new (&value);
}

static st_uint
large_integer_size (st_oop object)
{
    return (sizeof (struct st_large_integer) / sizeof (st_oop));
}

static void
large_integer_contents (st_oop object, struct contents *contents)
{
    contents->oops = NULL;
    contents->size = 0;
}

st_descriptor *
st_large_integer_descriptor (void)
{
    static st_descriptor __descriptor =
	{ .allocate         = allocate,
	  .allocate_arrayed = NULL,
	  .copy             = large_integer_copy,
	  .size             = large_integer_size,
	  .contents         = large_integer_contents,
	};

    return & __descriptor;
}
