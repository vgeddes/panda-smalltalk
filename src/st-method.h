/*
 * st-method.h
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

#ifndef __ST_METHOD_H__
#define __ST_METHOD_H__

#include <st-types.h>
#include <st-object.h>
#include <st-array.h>

#define ST_METHOD(oop) ((struct st_method *) (st_detag_pointer (oop)))

struct st_method
{
    struct st_header __parent__;

    st_oop header;
    st_oop bytecode;
    st_oop literals;
    st_oop selector;
};

typedef enum
{
    ST_METHOD_NORMAL,
    ST_METHOD_RETURN_RECEIVER,
    ST_METHOD_RETURN_INSTVAR,
    ST_METHOD_RETURN_LITERAL,
    ST_METHOD_PRIMITIVE,

} st_method_flags;

typedef enum
{
    ST_METHOD_LITERAL_NIL,
    ST_METHOD_LITERAL_TRUE,
    ST_METHOD_LITERAL_FALSE,
    ST_METHOD_LITERAL_MINUS_ONE,
    ST_METHOD_LITERAL_ZERO,
    ST_METHOD_LITERAL_ONE,
    ST_METHOD_LITERAL_TWO,

} st_method_literal_type;

#define ST_METHOD_HEADER(oop)   (ST_METHOD (oop)->header)
#define ST_METHOD_LITERALS(oop) (ST_METHOD (oop)->literals)
#define ST_METHOD_BYTECODE(oop) (ST_METHOD (oop)->bytecode)
#define ST_METHOD_SELECTOR(oop) (ST_METHOD (oop)->selector)

/*
 * CompiledMethod Header:
 *
 * The header is a smi containing various bitfields.
 *
 * A flag bitfield in the header indicates how the method must be executed. Generally
 * it provides various optimization hints to the interpreter. The flag bitfield
 * is alway stored in bits 31-29. The format of the remaining bits in the header
 * varies according to the flag value.  
 * 
 * flag (3 bits):
 *   0 : Activate method and execute its bytecodes 
 *   1 : The method simply returns 'self'. Has no side-effects. Don't bother creating a new activation.
 *   2 : The method simply returns an instance variable. Ditto.
 *   3 : The method simply returns a literal. Ditto.
 *   4 : The method performs a primitive operation.
 *
 * Bitfield format
 * 
 * flag = 0:
 *   [ flag: 3 | arg_count: 5 | temp_count: 6 | unused: 7 | large_context: 1 | primitive: 8 | tag: 2 ]
 *
 *   arg_count:      number of args
 *   temp_count:     number of temps
 *   stack_depth:    depth of stack
 *   primitive:      index of a primitive method
 *   tag:            The usual smi tag
 *
 * flag = 1:
 *   [ flag: 3 | unused: 27 | tag: 2 ]
 *
 * flag = 2:
 *   header: [ flag: 3 | unused: 11 | instvar: 16 | tag: 2 ]
 *
 *   instvar_index:  Index of instvar
 *
 * flag = 3:
 *   header: [ flag: 3 | unused: 23 | literal: 4 | tag: 2 ]
 *
 *   literal: 
 *      nil:             0
 *      true:            1
 *      false:           2
 *      -1:              3
 *       0:              4
 *       1:              5
 *       2:              6
 */

#define _ST_METHOD_SET_BITFIELD(bitfield, field, value) 	  		\
    ((bitfield) = ((bitfield) & ~(_ST_METHOD_##field##_MASK << _ST_METHOD_##field##_SHIFT)) \
	 | (((value) & _ST_METHOD_##field##_MASK) << _ST_METHOD_##field##_SHIFT))

#define _ST_METHOD_GET_BITFIELD(bitfield, field)			\
    (((bitfield) >> _ST_METHOD_##field##_SHIFT) & _ST_METHOD_##field##_MASK)

enum
{
    _ST_METHOD_FLAG_BITS           = 3,
    _ST_METHOD_ARG_BITS            = 5,
    _ST_METHOD_TEMP_BITS           = 6,
    _ST_METHOD_UNUSED_BITS         = 7,
    _ST_METHOD_LARGE_BITS          = 1,
    _ST_METHOD_INSTVAR_BITS        = 16,  
    _ST_METHOD_LITERAL_BITS        = 4,
    _ST_METHOD_PRIMITIVE_BITS      = 8,
    
    _ST_METHOD_PRIMITIVE_SHIFT     =  ST_TAG_SIZE,
    _ST_METHOD_INSTVAR_SHIFT       =  ST_TAG_SIZE,
    _ST_METHOD_LITERAL_SHIFT       =  ST_TAG_SIZE,
    _ST_METHOD_LARGE_SHIFT         = _ST_METHOD_PRIMITIVE_BITS + _ST_METHOD_PRIMITIVE_SHIFT,
    _ST_METHOD_UNUSED_SHIFT        = _ST_METHOD_LARGE_BITS     + _ST_METHOD_LARGE_SHIFT,
    _ST_METHOD_TEMP_SHIFT          = _ST_METHOD_UNUSED_BITS    + _ST_METHOD_UNUSED_SHIFT,
    _ST_METHOD_ARG_SHIFT           = _ST_METHOD_TEMP_BITS      + _ST_METHOD_TEMP_SHIFT,
    _ST_METHOD_FLAG_SHIFT          = _ST_METHOD_ARG_BITS       + _ST_METHOD_ARG_SHIFT,
    
    _ST_METHOD_PRIMITIVE_MASK      = ST_NTH_MASK (_ST_METHOD_PRIMITIVE_BITS),
    _ST_METHOD_LARGE_MASK          = ST_NTH_MASK (_ST_METHOD_LARGE_BITS),
    _ST_METHOD_INSTVAR_MASK        = ST_NTH_MASK (_ST_METHOD_INSTVAR_BITS),
    _ST_METHOD_LITERAL_MASK        = ST_NTH_MASK (_ST_METHOD_LITERAL_BITS),
    _ST_METHOD_TEMP_MASK           = ST_NTH_MASK (_ST_METHOD_TEMP_BITS),
    _ST_METHOD_ARG_MASK            = ST_NTH_MASK (_ST_METHOD_ARG_BITS),
    _ST_METHOD_FLAG_MASK           = ST_NTH_MASK (_ST_METHOD_FLAG_BITS),
};


static inline int
st_method_get_temp_count (st_oop method)
{
    return _ST_METHOD_GET_BITFIELD (ST_METHOD_HEADER (method), TEMP);
}

static inline int
st_method_get_arg_count (st_oop method)
{
    return _ST_METHOD_GET_BITFIELD (ST_METHOD_HEADER (method), ARG);
}

static inline int
st_method_get_large_context (st_oop method)
{
    return _ST_METHOD_GET_BITFIELD (ST_METHOD_HEADER (method), LARGE);
}

static inline int
st_method_get_primitive_index (st_oop method)
{
    return _ST_METHOD_GET_BITFIELD (ST_METHOD_HEADER (method), PRIMITIVE);
}

static inline st_method_flags
st_method_get_flags (st_oop method)
{   
    return _ST_METHOD_GET_BITFIELD (ST_METHOD_HEADER (method), FLAG);
}

static inline void
st_method_set_flags (st_oop method, st_method_flags flags)
{
    _ST_METHOD_SET_BITFIELD (ST_METHOD_HEADER (method), FLAG, flags);
}

static inline void
st_method_set_arg_count (st_oop method, int count)
{	
    _ST_METHOD_SET_BITFIELD (ST_METHOD_HEADER (method), ARG, count);
}

static inline void
st_method_set_temp_count (st_oop method, int count)
{
    _ST_METHOD_SET_BITFIELD (ST_METHOD_HEADER (method), TEMP, count);
}

static inline void
st_method_set_large_context (st_oop method, bool is_large)
{
    _ST_METHOD_SET_BITFIELD (ST_METHOD_HEADER (method), LARGE, is_large);
}

static inline void
st_method_set_primitive_index (st_oop method, int index)
{
    _ST_METHOD_SET_BITFIELD (ST_METHOD_HEADER (method), PRIMITIVE, index);
}

static inline void
st_method_set_instvar_index (st_oop method, int index)
{
    _ST_METHOD_SET_BITFIELD (ST_METHOD_HEADER (method), INSTVAR, index);
}

static inline void
st_method_set_literal_type (st_oop method, st_method_literal_type literal_type)
{
    _ST_METHOD_SET_BITFIELD (ST_METHOD_HEADER (method), LITERAL, literal_type);
}


static inline st_uchar *
st_method_bytecode_bytes (st_oop method)
{
    return st_byte_array_bytes (ST_METHOD_BYTECODE (method));    
}

#endif /* __ST_METHOD_H__ */
