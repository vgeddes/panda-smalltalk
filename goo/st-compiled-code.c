/*
 * st-compiled-code.c
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

#include "st-compiled-code.h"

/* CompiledMethod */

ST_DEFINE_VTABLE (st_compiled_method, st_heap_object_vtable ());


static bool
is_compiled_method (void)
{
    return true;
}

static void
st_compiled_method_vtable_init (STVTable * table)
{
    table->is_compiled_method = is_compiled_method;
}


/* CompiledBlock */

ST_DEFINE_VTABLE (st_compiled_block, st_heap_object_vtable ());

static bool
is_compiled_block (void)
{
    return true;
}

static void
st_compiled_block_vtable_init (STVTable * table)
{
    table->is_compiled_block = is_compiled_block;
}


